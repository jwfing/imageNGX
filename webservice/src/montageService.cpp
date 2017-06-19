#include "montageService.h"

#include <vector>
#include <sstream>

#include <boost/bind.hpp>

#include "common.h"
#include "stopWatch.h"

LoggerPtr MontageService::_logger = Logger::getLogger("cn.leancloud.imageService.MontageService");

const static string IMAGE_TYPE = "image/jpeg";

MontageService::MontageService(MemoryPool& memoryPool,
                               StoragePool& storagePool)
    : CommonService(memoryPool, storagePool) {
    LOG4CXX_INFO(_logger, "create montageService instance.");
}

MontageService::~MontageService() {
}

void MontageService::parseQuery(const HTTPRequestPtr& request,
                                string& bucket,
                                string& keys,
                                int& hostStyle,
                                int& width,
                                int& height,
                                int& margin,
                                int& cols,
                                string& background) {
    hostStyle = 0;
    width = height = -1;
    cols = -1;
    margin = -1;
    width = atoi(request->get_query(QUERY_WIDTH).c_str());
    height = atoi(request->get_query(QUERY_HEIGHT).c_str());
    hostStyle = atoi(request->get_query(QUERY_HOST_STYLE).c_str());
    bucket = request->get_query(QUERY_BUCKET);
    keys = request->get_query(QUERY_KEYS);
    cols = atoi(request->get_query(QUERY_COLS).c_str());
    margin = atoi(request->get_query(QUERY_MARGIN).c_str());
    background = request->get_query(QUERY_BACKGROUND);
}

void MontageService::operator()(const HTTPRequestPtr& request,
                                const TCPConnectionPtr& tcp_conn) {
    LOG4CXX_DEBUG(_logger, "enter operator() of montageService. thread=" << pthread_self());
    HTTPResponseWriterPtr writer(HTTPResponseWriter::create(tcp_conn, *request,
                                                            boost::bind(&TCPConnection::finish, tcp_conn)));
    string bucket, keys, background;
    int width, height, margin, cols, hostStyle;
    parseQuery(request, bucket, keys, hostStyle, width, height, margin, cols, background);
    if (keys.length() < 1
        || bucket.length() < 1
        || width <= 0
        || height <= 0
        || margin < 0
        || cols <= 0) {
        return HTTPServer::handle_bad_request(request, tcp_conn);
    }
    char delimiter = ',';
    stringstream ss(keys);
    string tmp;
    vector<Image*> images;
    while (getline(ss, tmp, delimiter)) {
        if (tmp.length() > 0) {
            Image* image = _storagePool.getImage(tmp.c_str(), bucket.c_str(), hostStyle);
            if (image != NULL) {
                LOG4CXX_INFO(_logger, "Add a new image with key:" << tmp << " and length:" << image->len());
                images.push_back(image);
            }
        }
    }
    int numOfImages = (int)images.size();
    if (numOfImages == 0) {
        return HTTPServer::handle_bad_request(request, tcp_conn);
    }
    vector<char*> srcs;
    vector<int> lenOfSrcs;
    int maxLength = 0;
    for (int i = 0; i < numOfImages; ++i) {
        int tmp;
        srcs.push_back((char*)images[i]->data(tmp));
        lenOfSrcs.push_back(tmp);
        if (maxLength < tmp) {
            maxLength = tmp;
        }
    }
    LOG4CXX_INFO(_logger, "ready to malloc memory: " << maxLength);
    MP_ID mpID = _memoryPool.alloc((int)(maxLength * 2));
    int64_t newBufferLen = 0;
    void* newBuffer = _memoryPool.getElementAddress(mpID, newBufferLen);
    if (NULL == newBuffer) {
        const char* errMsg = "crop failed. cause: can't alloc memory for new thumbnailer image";
        LOG4CXX_WARN(_logger, errMsg);
        for (int i = 0; i < numOfImages; ++i) {
            _storagePool.freeImage(images[i]);
        }
        return HTTPServer::handle_server_error(request, tcp_conn, errMsg);
    }
    int ret = _montager.montage(srcs, lenOfSrcs, numOfImages, (char*)newBuffer, newBufferLen, width, height, margin, cols, background);
    if (ret <= 0) {
        char errMsg[64];
        sprintf(errMsg, "montage failed, resultCode = %d", ret);
        LOG4CXX_WARN(_logger, errMsg);
        if (MP_NULL != mpID) {
            _memoryPool.freeNode(mpID);
            mpID = MP_NULL;
        }
        for (int i = 0; i < numOfImages; ++i) {
            _storagePool.freeImage(images[i]);
        }
        return HTTPServer::handle_server_error(request, tcp_conn, errMsg);
    } else {
        writeResponseHeader(writer, HTTPTypes::RESPONSE_CODE_OK, "", IMAGE_TYPE, ret, 0);
        writeResponseImage(writer, newBuffer, ret);
        writer->send();
        if (MP_NULL != mpID) {
            _memoryPool.freeNode(mpID);
            mpID = MP_NULL;
        }
        for (int i = 0; i < numOfImages; ++i) {
            _storagePool.freeImage(images[i]);
        }
    }
}
