#include "cropServiceWithS3.h"

#include <iostream>
#include <string.h>
#include <boost/bind.hpp>

#include "common.h"
#include "stopWatch.h"
#include "imagicImageOperator.h"
#include "imagickExtraImageInfo.h"

LoggerPtr CropServiceS3::_logger = Logger::getLogger("cn.leancloud.imageService.cropS3");
const static string IMAGE_TYPE = "image/jpeg";
const static string JSON_TYPE = "application/json";

CropServiceS3::CropServiceS3(Thumbnailer* thumbnailer,
                             MemoryPool& memoryPool,
                             StoragePool& storagePool)
    : CommonService(memoryPool, storagePool), _thumbnailer(thumbnailer) {
    LOG4CXX_INFO(_logger, "create cropService instance.");
}

CropServiceS3::~CropServiceS3() {
    _thumbnailer = NULL;
}

void CropServiceS3::parseQueryString(const string& queryString,
                                     int& w,
                                     int& h,
                                     int& hostStyle,
                                     int& extraInfo,
                                     ImageOperator* opers) {
    char* buffer = strdup(queryString.c_str());
    char* outer = NULL;
    char* buf = buffer;
    char* tmp = NULL;
    char* kvDelimiter = NULL;
    int maxParseTime = 10;
    if (NULL == buffer) {
        return;
    }
    hostStyle = extraInfo = 0;
    w = h = -1;
    while ((tmp = strtok_r(buf, "&", &outer)) != NULL && maxParseTime > 0) {
        maxParseTime--;
        buf = tmp;
        kvDelimiter = strcasestr(buf, "=");
        if (NULL == kvDelimiter) {
            continue;
        }
        *kvDelimiter = '\0';
        kvDelimiter++;
        if (strlen(buf) < 1 || strlen(kvDelimiter) < 1) {
            continue;
        }
        if (0 == strcasecmp(buf, QUERY_WIDTH)) {
            w = atoi(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_HEIGHT)) {
            h = atoi(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_HOST_STYLE)) {
            hostStyle = atoi(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_EXTRA_INFO)) {
            extraInfo = atoi(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_QUALITY)) {
            opers->setQuality(atoi(kvDelimiter));
        } else if (0 == strcasecmp(buf, QUERY_FORMAT)) {
            opers->setFmt(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_HUE)) {
            opers->setHue(atoi(kvDelimiter));
        } else if (0 == strcasecmp(buf, QUERY_SAT)) {
            opers->setSaturation(atoi(kvDelimiter));
        } else if (0 == strcasecmp(buf, QUERY_BRI)) {
            opers->setBrightness(atoi(kvDelimiter));
        } else if (0 == strcasecmp(buf, QUERY_CROP)) {
            opers->setCrop(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_EXP)) {
            opers->setExposure(atoi(kvDelimiter));
        } else if (0 == strcasecmp(buf, QUERY_BLUR)) {
            opers->setBlur(atoi(kvDelimiter));
        } else if (0 == strcasecmp(buf, QUERY_CLIPS)) {
            opers->setRainbows(pion::algorithm::url_decode(kvDelimiter));
        } else if (0 == strcasecmp(buf, QUERY_RAINBOW_HEIGHT)) {
            opers->setRainbowHeight(atoi(kvDelimiter));
        }
        buf = NULL;
    }
}

bool CropServiceS3::shouldOriginalImage(int w, int h, ImageOperator* opers) {
    if (w == -1 && h == -1
        && opers->getFmt().empty()
        && is_equal(opers->getQuality(), -1)
        && is_equal(opers->getSaturation(), -1)
        && is_equal(opers->getHue(), -1)
        && is_equal(opers->getBrightness(), -1)
        && is_equal(opers->getExposure(), -1)
        && is_equal(opers->getBlur(), -1)
        && opers->getCrop().empty()) {
        return true;
    }

    return false;
}

int CropServiceS3::generateFileName(char* buffer,
                                    int bufferLen,
                                    const string& key,
                                    int width,
                                    int height,
                                    ImageOperator* opers,
                                    bool shouldOriginalImage) {
    if (shouldOriginalImage) {
        return snprintf(buffer, bufferLen, "%s", key.c_str());
    } else {
        return snprintf(buffer, bufferLen, "%s-%s-%d-%d-%d-%d-%d-%d-%d-%d",
                        key.c_str(),
                        opers->getFmt().c_str(),
                        width, height,
                        opers->getQuality(),
                        (int)opers->getHue(),
                        (int)opers->getSaturation(),
                        (int)opers->getBrightness(),
                        (int)opers->getExposure(),
                        (int)opers->getBlur());
    }
}

string CropServiceS3::getOriginalFileUrl(const string& host,
                                         const string& bucket,
                                         const string& filename,
                                         int uriStyle) {
    string protocol = "https://";
    if (uriStyle == 1) {
        // virtual domain
        return protocol + bucket + "." + host + "/" + filename;
    } else {
        return protocol + host + "/" + bucket + "/" + filename;
    }
}

void CropServiceS3::sendImage(HTTPResponseWriterPtr writer,
                              Image* result,
                              const string& fileName) {
    int dataLen = 0;
    const void* data = result->data(dataLen);
    cout << "datalen:" << dataLen << endl;
    writeResponseHeader(writer, HTTPTypes::RESPONSE_CODE_OK, fileName, IMAGE_TYPE, dataLen, result->getTime());
    writeResponseImage(writer, data, dataLen);
    writer->send();
}

int CropServiceS3::doThumbnail(const char* src, size_t lenOfSrc,
                               char* desc, size_t lenOfDesc,
                               const Rect& rectOfInterest,
                               int width, int height,
                               ImageOperator* opers,
                               ExtraImageInfo* extraImageInfo) {
    int thumbResult = -1;
    try {
        StopWatch cropWatch(CROP_IMAGE);
        thumbResult = _thumbnailer->create(src, lenOfSrc, desc, lenOfDesc,
                                           rectOfInterest, width, height,
                                           opers, extraImageInfo);
    } catch (...) {
        thumbResult = EC_CONVERT_FAILED;
    }
    return thumbResult;
}

void CropServiceS3::handleNotFound(HTTPResponseWriterPtr writer,
                                   const string& bucket,
                                   int hostStyle,
                                   int targetWidth,
                                   int targetHeight) {
    string url = "";
    url = getOriginalFileUrl("s3.amazonaws.com", bucket, CONF_NOT_FOUND_IMAGE, hostStyle);
    redirectResponse(writer, url);
}

void CropServiceS3::operator()(const HTTPRequestPtr& request,
                               const TCPConnectionPtr& tcp_conn) {
    LOG4CXX_DEBUG(_logger, "enter operator() of cropService. thread=" << pthread_self());
    StopWatch totalWatch(WEB_SERVING);
    // for log
    int64_t startTime = get_cur_microseconds_time();

    if (NULL == _thumbnailer) {
        return HTTPServer::handle_server_error(request, tcp_conn, "Thumbernail Service is not ready");
    }

    string bucket, key;
    if (!parseResource(request, bucket, key)) {
        return HTTPServer::handle_bad_request(request, tcp_conn);
    }

    const string queryString = getQueryString(request);
    int w, h, hostStyle, extraInfo;
    ImageOperator* opers = new ImagicImageOperator();
    parseQueryString(queryString, w, h, hostStyle, extraInfo, opers);
    HTTPResponseWriterPtr writer(HTTPResponseWriter::create(tcp_conn, *request,
                                                            boost::bind(&TCPConnection::finish, tcp_conn)));
    if (key.length() < 1 || bucket.length() < 1) {
        if (opers != NULL) {
            delete opers;
            opers = NULL;
        }
        return HTTPServer::handle_bad_request(request, tcp_conn);
    }
    char fileName[1024] = {0};
    bool isOriginal = shouldOriginalImage(w, h, opers);
    int res = generateFileName(fileName, 1024, key, w, h,
                               opers,
                               isOriginal);
    if (res < 0) {
        if (opers != NULL) {
            delete opers;
            opers = NULL;
        }
        return HTTPServer::handle_bad_request(request, tcp_conn);
    }
    Image* result = NULL;
    // FIXME: only original image will get the image first
    if (isOriginal) {
        // sendImage(writer, result, fileName);
        string url = getOriginalFileUrl("s3.amazonaws.com", bucket, fileName, hostStyle);
        redirectResponse(writer, url);
        // _storagePool.freeImage(result);
        LOG4CXX_INFO(_logger, "[Redirect:Original-file] successful response for urlMD5=" << fileName);
        if (opers != NULL) {
            delete opers;
            opers = NULL;
        }
        return;
    } else {
        // need crop
        string originFile = key;
        {
            StopWatch readWatch(READ_FROM_STORE);
            int64_t readStartTime = get_cur_microseconds_time();
            result = _storagePool.getImage(originFile.c_str(), bucket.c_str(), hostStyle);
            LOG4CXX_INFO(_logger, "read source image from "
                         << bucket
                         << " costs "
                         << ((get_cur_microseconds_time() - readStartTime) / 1000)
                         << "ms");
        }
        if (NULL == result) {
            if (opers != NULL) {
                delete opers;
                opers = NULL;
            }
            handleNotFound(writer, bucket, hostStyle, w, h);
            return;
        }

        int origDataLen = 0;
        const void* origData = result->data(origDataLen);
        // alloc memory for thumbnail
        MP_ID mpID = _memoryPool.alloc((int)(origDataLen*1.5));
        int64_t newBufferLen = 0;
        void* newBuffer = _memoryPool.getElementAddress(mpID, newBufferLen);

        if (NULL == newBuffer) {
            // can't get memory.
            const char* errMsg = "crop failed. cause: can't alloc memory for new thumbnailer image";
            LOG4CXX_WARN(_logger, errMsg);
            _storagePool.freeImage(result);
            if (opers != NULL) {
                delete opers;
                opers = NULL;
            }
            return HTTPServer::handle_server_error(request, tcp_conn, errMsg);
        }

        int thumbResult = -1;
        Rect rect(-1, -1, -1, -1);
        ExtraImageInfo* extraImageInfo = NULL;
        if (extraInfo) {
            extraImageInfo = new ImagickExtraImageInfo();
        }
        thumbResult = doThumbnail((const char*)origData, origDataLen,
                                  (char*)newBuffer, newBufferLen,
                                  rect, w, h, opers, extraImageInfo);
        if (EC_BUFFER_NOT_ENOUGH == thumbResult) {
            _memoryPool.freeNode(mpID);
            mpID = _memoryPool.alloc(newBufferLen * 2);
            LOG4CXX_INFO(_logger, "need more large size for resizing. origin=" << origDataLen << ", firstPhrase=" << newBufferLen);
            newBuffer = _memoryPool.getElementAddress(mpID, newBufferLen);
            if (NULL != newBuffer) {
                thumbResult = doThumbnail((const char*)origData, origDataLen,
                                          (char*)newBuffer, newBufferLen,
                                          rect, w, h, opers, extraImageInfo);
            }
        }
        if (0 >= thumbResult) {
            LOG4CXX_WARN(_logger, "crop failed and send the original image. cause: failed to create thumbnailer, resultCode=" << thumbResult);
            sendImage(writer, result, originFile);
            if (MP_NULL != mpID) {
                _memoryPool.freeNode(mpID);
                mpID = MP_NULL;
            }
        } else {
            // add extra info to response header
            if (extraInfo && extraImageInfo != NULL) {
                string extraResponse = "{" + extraImageInfo->getResult() + "}";
                size_t length = extraResponse.length();
                writeResponseHeader(writer, HTTPTypes::RESPONSE_CODE_OK, fileName, JSON_TYPE, (int)length, 0);
                writer->write((void*) extraResponse.c_str(), length);
                writer->send();
            } else {
                writeResponseHeader(writer, HTTPTypes::RESPONSE_CODE_OK, fileName, IMAGE_TYPE, (int)thumbResult, 0);
                writeResponseImage(writer, newBuffer, thumbResult);
                writer->send();
            }
            // int saveRes = _storagePool.saveImage(fileName, newBuffer, (int)thumbResult, mpID, bucket.c_str());
            // if (0 != saveRes) {
                _memoryPool.freeNode(mpID);
                mpID = MP_NULL;
                // }
                LOG4CXX_INFO(_logger, "[RESIZE] successful response for urlMD5=" << fileName
                             << " for bucket=" << bucket << " in " << ((get_cur_microseconds_time() - startTime)/1000)
                             << "ms");
        }
        _storagePool.freeImage(result);
        if (opers != NULL) {
            delete opers;
            opers = NULL;
        }
        if (extraImageInfo != NULL) {
            delete extraImageInfo;
            extraImageInfo = NULL;
        }
    }
}
