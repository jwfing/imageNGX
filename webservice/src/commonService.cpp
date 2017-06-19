#include "commonService.h"

CommonService::CommonService(MemoryPool& memoryPool, StoragePool& storagePool): _memoryPool(memoryPool), _storagePool(storagePool) {

}

bool CommonService::parseResource(const pion::http::request_ptr& request, std::string& bucket, std::string& key) {
    const string resource(get_relative_resource(request->get_resource()));
    int maxParseTime = 10;
    char* buffer = strdup(resource.c_str());
    char* buf = buffer;
    char* savePtr = NULL;
    char* tmp = NULL;
    bucket = "";
    key = "";
    if (NULL == buffer) {
        return false;
    }
    int cnt = 0;
    while ((tmp = strtok_r(buf, "/", &savePtr)) != NULL && cnt < maxParseTime) {
        cnt ++;
        if (cnt == 1) {
            bucket = tmp;
        } else if (cnt == 2) {
            key = tmp;
        } else if (cnt > 2) {
            key = key + "/" + tmp;
        }
        buf = NULL;
    }
    if (cnt >= maxParseTime) {
        // Wrong mapping
        return false;
    }
    key = pion::algorithm::url_decode(key);
    return true;
}

// write response header
void CommonService::writeResponseHeader(http::response_writer_ptr writer, const unsigned int& statusCode,
                         const string& etag, const string& contentType, int contentLength, int64_t lastmodify) {
    writer->get_response().set_status_code(statusCode);
    if (http::types::RESPONSE_CODE_OK == statusCode) {
        if (lastmodify <= 0) {
            lastmodify = get_cur_microseconds_time() / 1000000;
        }
        writer->get_response().set_content_type(contentType);
        writer->get_response().set_content_length(contentLength);
        writer->get_response().add_header("Cache-Control", "max-age=36000000");
        writer->get_response().set_last_modified(lastmodify);
        writer->get_response().add_header("Expires", http::types::get_date_string(lastmodify + 36000000));
    } else if (http::types::RESPONSE_CODE_NOT_FOUND == statusCode) {
        writer->get_response().set_content_type(http::types::CONTENT_TYPE_HTML);
        writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_NOT_FOUND);
    } else if (http::types::RESPONSE_CODE_BAD_REQUEST == statusCode) {
        writer->get_response().set_content_type(http::types::CONTENT_TYPE_HTML);
        writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_BAD_REQUEST);
    } else {
        writer->get_response().set_content_type(http::types::CONTENT_TYPE_HTML);
        writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_SERVER_ERROR);
    }
}

// write reponse body
void CommonService::writeResponseImage(http::response_writer_ptr writer, const void* data, int dataLen) {
    writer->write((void*)data, (size_t)dataLen);
}

void CommonService::redirectResponse(HTTPResponseWriterPtr writer,
                                      const string& redirectUrl) {
    writer->get_response().set_status_code(HTTPTypes::RESPONSE_CODE_FOUND);
    writer->get_response().set_status_message(HTTPTypes::RESPONSE_MESSAGE_FOUND);
    writer->get_response().add_header(HTTPTypes::HEADER_LOCATION, redirectUrl.c_str());
    writer->get_response().add_header("Cache-Control",
                                    "no-cache");
    writer->send();
}

void CommonService::sendNotFoundResponse(const http::request_ptr& http_request_ptr,
                                       const tcp::connection_ptr& tcp_conn)
{
    static const std::string NOT_FOUND_HTML_START =
        "<html><head>\n"
        "<title>404 Not Found</title>\n"
        "</head><body>\n"
        "<h1>Not Found</h1>\n"
        "<p>The requested URL ";
    static const std::string NOT_FOUND_HTML_FINISH =
        " was not found on this server.</p>\n"
        "</body></html>\n";
    http::response_writer_ptr writer(http::response_writer::create(tcp_conn, *http_request_ptr,
                                 boost::bind(&tcp::connection::finish, tcp_conn)));
    writer->get_response().set_status_code(http::types::RESPONSE_CODE_NOT_FOUND);
    writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_NOT_FOUND);
    if (http_request_ptr->get_method() != http::types::REQUEST_METHOD_HEAD) {
        writer->write_no_copy(NOT_FOUND_HTML_START);
        writer << algorithm::xml_encode(http_request_ptr->get_resource());
        writer->write_no_copy(NOT_FOUND_HTML_FINISH);
    }
    writer->send();
}
