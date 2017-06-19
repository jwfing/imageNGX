#include "cropService.h"

#include <iostream>
#include <string.h>
#include <boost/bind.hpp>
#include <pion/algorithm.hpp>
#include <pion/user.hpp>
#include <pion/http/types.hpp>

#include "common.h"
#include "stopWatch.h"
#include "defaultFavicon.h"

LoggerPtr CropService::_logger = Logger::getLogger("cn.leancloud.imageService.crop");

const static string ICON_TYPE = "image/x-icon";
const static string IMAGE_TYPE = "image/jpeg";

CropService::CropService(Thumbnailer* thumbnailer, MemoryPool& memoryPool, StoragePool& storagePool)
    : CommonService(memoryPool, storagePool), _thumbnailer(thumbnailer) {
    LOG4CXX_INFO(_logger, "create cropService instance.");
}

CropService::~CropService() {
    _thumbnailer = NULL;
}

int CropService::generateFileName(char* buffer, int bufferLen, const string& md5, int width,
                int height, int x, int y, int x2, int y2) {
    return snprintf(buffer, bufferLen, "%s-%d*%d-%d%d%d%d", md5.c_str(), width, height, x, y, x2, y2);
}

bool CropService::isNeedDecode(const char* url) {
    const char* unencodeEqual = "=";
    if (NULL == strstr(url, unencodeEqual)) {
        return true;
    }
    return false;
}

char* CropService::decodeUrl(char* url) {
    if (NULL == url) {
        return NULL;
    }
    std::string result = pion::algorithm::url_decode(std::string(url));
    free(url);
    url = strdup(result.c_str());
    return url;
}

void CropService::parseQueryString(const string& original, string& urlMD5, string& domain,
    int& width, int& height, int& x, int& y, int& x2, int& y2) {
    char* buffer = strdup(original.c_str());
    char* outer = NULL;
    char* buf = buffer;
    char* tmp = NULL;
    char* kvDelimiter = NULL;
    int maxParseTime = 10;
    urlMD5 = "";
    domain = "";
    x = y = x2 = y2 = width = height = -1;
    if (NULL == buffer) {
        return;
    }

    if (isNeedDecode(buffer)) {
        buffer = decodeUrl(buffer);
        buf = buffer;
    }
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
        if (0 == strcasecmp(buf, QUERY_URLMD5)) {
            urlMD5 = kvDelimiter;
        } else if (0 == strcasecmp(buf, QUERY_DOMAIN)) {
            domain = kvDelimiter;
        } else if (0 == strcasecmp(buf, QUERY_WIDTH)) {
            width = atoi(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_HEIGHT)) {
            height = atoi(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_X)) {
            x = atoi(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_X2)) {
            x2 = atoi(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_Y)) {
            y = atoi(kvDelimiter);
        } else if (0 == strcasecmp(buf, QUERY_Y2)) {
            y2 = atoi(kvDelimiter);
        }
        buf = NULL;
    }
    free(buffer);
}

// CropService member functions
// handles requests for CropService
void CropService::operator()(const pion::http::request_ptr& request,
                             const pion::tcp::connection_ptr& tcp_conn) {
    LOG4CXX_DEBUG(_logger, "enter operator() of cropService. thread=" << pthread_self());
    StopWatch totalWatch(WEB_SERVING);

    http::response_writer_ptr writer(http::response_writer::create(tcp_conn, *request,
                                                            boost::bind(&tcp::connection::finish, tcp_conn)));

    const string queryString = request->get_query_string();
    std::vector<std::string> queryTerms;
    string urlMD5 = "";
    string domain = "";
    int w, h, x, x2, y, y2;
    w = h = x = x2 = y = y2 = -1;
    parseQueryString(queryString, urlMD5, domain, w, h, x, y, x2, y2);
    LOG4CXX_DEBUG(_logger, "queryString details. origin:" << queryString
        << ", urlmd5=" << urlMD5 << ", d=" << domain << ", w=" << w << ", h=" << h << ", x=" << x
        << ", y=" << y << ", x2=" << x2 << ", y2=" << y2);
    if (domain.length() > 0) {
        Image* result = NULL;
        std::reverse(domain.begin(), domain.end());
        {
            StopWatch readWatch(READ_FROM_STORE);
            result = _storagePool.getImage(domain.c_str(), "");
        }
        if (NULL != result) {
            // has existed, output directly
            int dataLen = 0;
            const void* data = result->data(dataLen);
            writeResponseHeader(writer, http::types::RESPONSE_CODE_OK, domain, ICON_TYPE, dataLen, result->getTime());
            writeResponseImage(writer, data, dataLen);
            // send the writer
            writer->send();
            _storagePool.freeImage(result);
            LOG4CXX_DEBUG(_logger, "[EXISTED] successful response for domain=" << domain);
        } else {
            DefaultFavicon* df = DefaultFavicon::getInstance();
            size_t length = 0;
            void* buffer = df->data(length);
            if (NULL  == buffer || length == 0) {
                LOG4CXX_WARN(_logger, "crop failed. original file isn't existed. domain=" << domain);
                writeResponseHeader(writer, http::types::RESPONSE_CODE_NOT_FOUND, "", "", 0, 0);
                writer->send();
            } else {
                LOG4CXX_INFO(_logger, "[DEFAULT_FAVICON] can't retrieve favicon for domain:" << domain << ", and use default icon.");
                writeResponseHeader(writer, http::types::RESPONSE_CODE_OK, domain, "", (int)length, 0);
                writeResponseImage(writer, buffer, (int)length);
                // send the writer
                writer->send();
            }
        } 
    } else if (urlMD5.length() < 1 || ((w == -1) && (h == -1))) {
        // query string is invalid, bad request
        LOG4CXX_WARN(_logger, "crop failed. query is invalid. urlMD5=" << urlMD5 << ", w=" << w << ", h=" << h);
        writeResponseHeader(writer, http::types::RESPONSE_CODE_BAD_REQUEST, "", "", 0, 0);
        // send the writer
        writer->send();
    } else if (NULL == _thumbnailer) {
        // internal error
        LOG4CXX_WARN(_logger, "crop failed. thumbnailer instance is NULL");
        writeResponseHeader(writer, http::types::RESPONSE_CODE_SERVER_ERROR, "", "", 0, 0);
        // send the writer
        writer->send();
    } else {
        char fileName[1024] = {0};
        int res = generateFileName(fileName, 1024, urlMD5, w, h, x, y, x2, y2);
        if (res < 0) {
            // urlMD5 exceed limit? bad request
            LOG4CXX_WARN(_logger, "crop failed. query is invalid. urlMD5=" << urlMD5 << ", w=" << w << ", h=" << h);
            writeResponseHeader(writer, http::types::RESPONSE_CODE_BAD_REQUEST, "", "", 0, 0);
            writer->send();
        } else {
            Image* result = NULL;
            {
                StopWatch readWatch(READ_FROM_STORE);
                result = _storagePool.getImage(fileName, "");
            }
            if (NULL != result) {
                // has existed, output directly
                int dataLen = 0;
                const void* data = result->data(dataLen);
                writeResponseHeader(writer, http::types::RESPONSE_CODE_OK, fileName, IMAGE_TYPE, dataLen, result->getTime());
                writeResponseImage(writer, data, dataLen);
                writer->send();
                _storagePool.freeImage(result);
                LOG4CXX_INFO(_logger, "[EXISTED] successful response for urlMD5=" << fileName);
            } else {
                // new crop
                string originFile = urlMD5 + "-orig";
                {
                    StopWatch readWatch(READ_FROM_STORE);
                    result = _storagePool.getImage(originFile.c_str(), "");
                    int retry = 0;
                    while (NULL == result && retry < 3) {
                        result = _storagePool.getImage(originFile.c_str(), "");
                        retry++;
                    }
                }
                if (NULL == result) {
                    // original image doesn't existed. FILE_NOT_FOUND
                    LOG4CXX_WARN(_logger, "crop failed. original file isn't existed. fileName=" << originFile);
                    writeResponseHeader(writer, http::types::RESPONSE_CODE_NOT_FOUND, "", "", 0, 0);
                    writer->send();
                } else {
                    int origDataLen = 0;
                    const void* origData = result->data(origDataLen);
                    // alloc memory for thumbnail
                    MP_ID mpID = _memoryPool.alloc((int)(origDataLen*1.1));
                    int64_t newBufferLen = 0;
                    void* newBuffer = _memoryPool.getElementAddress(mpID, newBufferLen);
                    if (NULL == newBuffer) {
                        // can't get memory.
                        LOG4CXX_WARN(_logger, "crop failed. cause: can't alloc memory for new thumbnailer image");
                        writeResponseHeader(writer, http::types::RESPONSE_CODE_NOT_FOUND, "", "", 0, 0);
                        writer->send();
                    } else {
                        int thumbResult = -1;
                        try {
                            Rect rect(x, y, x2, y2);
                            StopWatch cropWatch(CROP_IMAGE);
                            thumbResult = _thumbnailer->create((const char*)origData, origDataLen, (char*)newBuffer, newBufferLen, rect, w, h);
                        } catch (...) {
                            thumbResult = EC_CONVERT_FAILED;
                        }
                        if (EC_BUFFER_NOT_ENOUGH == thumbResult) {
                            _memoryPool.freeNode(mpID);
                            mpID = _memoryPool.alloc(newBufferLen*2);
                            LOG4CXX_INFO(_logger, "need more large size for resizing. origin=" << origDataLen << ", firstPhrase=" << newBufferLen);
                            newBuffer = _memoryPool.getElementAddress(mpID, newBufferLen);
                            if (NULL != newBuffer) {
                                try {
                                    Rect rect(x, y, x2, y2);
                                    StopWatch cropWatch(CROP_IMAGE);
                                    thumbResult = _thumbnailer->create((const char*)origData, origDataLen, (char*)newBuffer, newBufferLen, rect, w, h);
                                } catch (...) {
                                    thumbResult = EC_CONVERT_FAILED;
                                }
                            }
                        }
                        if (0 >= thumbResult) {
                            // error
                            LOG4CXX_WARN(_logger, "crop failed and send the original image. cause: failed to create thumbnailer, resultCode=" << thumbResult);
                            writeResponseHeader(writer, http::types::RESPONSE_CODE_OK, fileName, IMAGE_TYPE, origDataLen, result->getTime());
                            writeResponseImage(writer, origData, origDataLen);
                            writer->send();
                            if (MP_NULL != mpID) {
                                _memoryPool.freeNode(mpID);
                                mpID = MP_NULL;
                            }
                        } else {
                            // create a new thumbnail.
                            writeResponseHeader(writer, http::types::RESPONSE_CODE_OK, fileName, IMAGE_TYPE, (int)thumbResult, 0);
                            writeResponseImage(writer, newBuffer, thumbResult);
                            writer->send();
                            int saveRes = _storagePool.saveImage(fileName, newBuffer, (int)thumbResult, mpID, "");
                            if (0 != saveRes) {
                                _memoryPool.freeNode(mpID);
                                mpID = MP_NULL;
                            }
                            LOG4CXX_INFO(_logger, "[RESIZE] successful response for urlMD5=" << fileName);
                        }
                    }
                    _storagePool.freeImage(result);
                }
            }
        }
    }
}

