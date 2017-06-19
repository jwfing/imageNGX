#include "rainbowService.h"

#include <vector>
#include <sstream>

#include <boost/bind.hpp>

#include "common.h"
#include "stopWatch.h"

LoggerPtr RainbowService::_logger = Logger::getLogger("cn.leancloud.imageService.RainbowService");

const static string IMAGE_TYPE = "image/gif";

RainbowService::RainbowService(MemoryPool& memoryPool,
                               StoragePool& storagePool)
    : CommonService(memoryPool, storagePool) {
    LOG4CXX_INFO(_logger, "create rainbowService instance.");
}

RainbowService::~RainbowService(){}

void RainbowService::parseQuery(const HTTPRequestPtr& request,
                                vector<int>& clips,
                                int& height) {
    height = 0;
    clips.clear();
    height = atoi(request->get_query(QUERY_HEIGHT).c_str());
    string strClips = request->get_query(QUERY_CLIPS);
    if (!strClips.empty()) {
        char delimiter = ',';
        stringstream ss(strClips);
        string tmp;
        while (getline(ss, tmp, delimiter)) {
            if (tmp.length() > 0) {
                int clipLength = atoi(tmp.c_str());
                if (clipLength > 0)
                    clips.push_back(clipLength);
            }
        }
    }
}

void RainbowService::operator()(const HTTPRequestPtr& request,
                                const TCPConnectionPtr& tcp_conn) {
    LOG4CXX_DEBUG(_logger, "enter operator() of rainbowService. thread=" << pthread_self());
    HTTPResponseWriterPtr writer(HTTPResponseWriter::create(tcp_conn, *request,
                                                            boost::bind(&TCPConnection::finish, tcp_conn)));

    vector<int> clips;
    int height;
    parseQuery(request, clips, height);
    if (clips.empty() || height <= 0 ) {
        return HTTPServer::handle_bad_request(request, tcp_conn);
    }
    MP_ID mpID = _memoryPool.alloc((int)(1024 * 10));
    int64_t newBufferLen = 0;
    void* newBuffer = _memoryPool.getElementAddress(mpID, newBufferLen);
    if (NULL == newBuffer) {
        const char* errMsg = "rainbow failed. cause: can't alloc memory for image";
        LOG4CXX_WARN(_logger, errMsg);
        return HTTPServer::handle_server_error(request, tcp_conn, errMsg);
    }
    int ret= _rainbow.createRainbow(height, clips, (char *)newBuffer, newBufferLen);
    if (ret <= 0) {
        char errMsg[64];
        sprintf(errMsg, "rainbow failed, resultCode = %d", ret);
        LOG4CXX_WARN(_logger, errMsg);
        if (MP_NULL != mpID) {
            _memoryPool.freeNode(mpID);
            mpID = MP_NULL;
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
    }
}


