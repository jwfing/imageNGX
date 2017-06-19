#include "statusService.h"

#include <iostream>
#include <boost/bind.hpp>

#include "common.h"
#include "stopWatch.h"
#include "misc.h"

LoggerPtr StatusService::_logger = Logger::getLogger("cn.leancloud.imageService.status");

StatusService::StatusService(MemoryPool& memoryPool, StoragePool& storagePool)
    : _memoryPool(memoryPool), _storagePool(storagePool) {
    LOG4CXX_INFO(_logger, "create statusService instance.");
    _startTime = get_cur_microseconds_time();
}

StatusService::~StatusService() {
}

/// used by handleRequest to write dictionary terms
/*
void writeDictionaryTerm(HTTPResponseWriterPtr& writer,
                         const HTTPTypes::QueryParams::value_type& val,
                         const bool decode)
{
    // text is copied into writer text cache
    writer << val.first << HTTPTypes::HEADER_NAME_VALUE_DELIMITER
    << (decode ? algo::url_decode(val.second) : val.second)
    << HTTPTypes::STRING_CRLF;
}
*/

string StatusService::stat() {
    static char buffer[256] = {0};
    int64_t cur = get_cur_microseconds_time();
    int64_t running_time = (cur - _startTime) / 1000 / 1000;
    int day = (int)(running_time / 86400);
    running_time = running_time % 86400;
    int hour = (int)(running_time / 3600);
    running_time = running_time % 3600;
    int min = (int)(running_time / 60);
    int sec = (int)(running_time % 60);

    snprintf(buffer, 256, "%d day %d hours %d minutes %d seconds\n", day, hour, min, sec);
    return string(buffer);
}

// StatusService member functions
// handles requests for StatusService
void StatusService::operator()(const HTTPRequestPtr& request, const TCPConnectionPtr& tcp_conn) {
    LOG4CXX_INFO(_logger, "enter operator() of statusService. thread=" << pthread_self());

    // Notice: following just a sample for requestHandler
    // and we need to replace with the actual logic.

    // this web service uses static text to test the mixture of "copied" with
    // "static" (no-copy) text
    static const std::string MEMORY_INFO_TEXT("[MEMORY Info]");
    static const std::string STORAGE_INFO_TEXT("[STORAGE Info]");
    static const std::string SERVING_INFO_TEXT("[SERVING Info]");
    static const std::string SYSTEM_INFO_TEXT("[SYSTEM Info]");

    // Set Content-type to "text/plain" (plain ascii text)
    HTTPResponseWriterPtr writer(HTTPResponseWriter::create(tcp_conn, *request,
                                                            boost::bind(&TCPConnection::finish, tcp_conn)));
    writer->get_response().set_content_type(HTTPTypes::CONTENT_TYPE_TEXT);

    writer->write_no_copy(SYSTEM_INFO_TEXT);
    writer->write_no_copy(HTTPTypes::STRING_CRLF);
    writer->write_no_copy(HTTPTypes::STRING_CRLF);
    writer << stat()
        << HTTPTypes::STRING_CRLF
        << HTTPTypes::STRING_CRLF;

    writer->write_no_copy(MEMORY_INFO_TEXT);
    writer->write_no_copy(HTTPTypes::STRING_CRLF);
    writer->write_no_copy(HTTPTypes::STRING_CRLF);
    writer << _memoryPool.stat()
        << HTTPTypes::STRING_CRLF
        << HTTPTypes::STRING_CRLF;

    writer->write_no_copy(STORAGE_INFO_TEXT);
    writer->write_no_copy(HTTPTypes::STRING_CRLF);
    writer->write_no_copy(HTTPTypes::STRING_CRLF);
    writer << _storagePool.stat()
        << HTTPTypes::STRING_CRLF
        << HTTPTypes::STRING_CRLF;

    string servingStat = MetricsSink::getInstance().stat();
    writer->write_no_copy(SERVING_INFO_TEXT);
    writer->write_no_copy(HTTPTypes::STRING_CRLF);
    writer->write_no_copy(HTTPTypes::STRING_CRLF);
    writer << servingStat
        << HTTPTypes::STRING_CRLF
        << HTTPTypes::STRING_CRLF;

    // send the writer
    writer->send();
}

