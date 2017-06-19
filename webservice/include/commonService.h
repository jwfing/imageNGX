#ifndef CN_LEANCLOUD_IMAGE_SERVICE_COMMON_SERVICE_H_
#define CN_LEANCLOUD_IMAGE_SERVICE_COMMON_SERVICE_H_

#include "pion_common.h"

#include <string>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include "storagePool.h"
#include "memoryPool.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

using namespace std;

class CommonService : public pion::http::plugin_service {
public:
	CommonService(MemoryPool& memoryPool, StoragePool& storagePool);
	virtual ~CommonService(){};

protected:
	bool parseResource(const pion::http::request_ptr& request, std::string& bucket, std::string& resourceId);

	inline const std::string getQueryString(const pion::http::request_ptr& request) const{
		return request->get_query_string();
	};
	inline const std::string getQuery(const pion::http::request_ptr& request, const std::string& key) const {
		return request->get_query(key);
	};
	inline const std::string& get_method(const pion::http::request_ptr& request) const {
	    return request->get_method();
	};

    // write response header
    void writeResponseHeader(http::response_writer_ptr writer, const unsigned int& statusCode,
                const string& etag, const string& contentType, int contentLength, int64_t lastmodify);
    // write reponse body
    void writeResponseImage(http::response_writer_ptr writer, const void* data, int dataLen);
    // redirect
    void redirectResponse(HTTPResponseWriterPtr writer, const string& redirectUrl);
    // send not found response
    void sendNotFoundResponse(const pion::http::request_ptr& http_request_ptr,
                              const pion::tcp::connection_ptr& tcp_conn);

protected:
    MemoryPool& _memoryPool;     // reference of memory pool
    StoragePool& _storagePool;   // reference of storage pool
};

#endif