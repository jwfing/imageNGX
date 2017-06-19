// ---------------------------------------------------------------------
// pion:  a Boost C++ framework for building lightweight HTTP interfaces
// ---------------------------------------------------------------------
// Copyright (C) 2007-2014 Splunk Inc.  (https://github.com/splunk/pion)
//
// Distributed under the Boost Software License, Version 1.0.
// See http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include "fileService.h"
#include <pion/error.hpp>
#include <pion/plugin.hpp>
#include <pion/algorithm.hpp>
#include <pion/http/response_writer.hpp>

using namespace pion;
using namespace std;

// FileService member functions
FileService::FileService(MemoryPool& memoryPool, StoragePool& storagePool)
    : CommonService(memoryPool, storagePool), m_logger(PION_GET_LOGGER("pion.FileService"))
{}

void FileService::operator()(const http::request_ptr& http_request_ptr, const tcp::connection_ptr& tcp_conn)
{
    // get the relative resource path for the request
    string bucket, filename;
    if (!parseResource(http_request_ptr, bucket, filename)) {
        // bad request.
        sendNotFoundResponse(http_request_ptr, tcp_conn);
        return;
    }
    const std::string relative_path(get_relative_resource(http_request_ptr->get_resource()));

    PION_LOG_INFO(m_logger, "resource=" << http_request_ptr->get_resource() << ", relative_path="
        << relative_path << ", queryString=" << http_request_ptr->get_query_string());
    // determine the path of the file being requested
    PION_LOG_INFO(m_logger, "target bucket:" << bucket << ", file: " << filename);
             // prepare a response and set the Content-Type
            http::response_writer_ptr writer(http::response_writer::create(tcp_conn, *http_request_ptr,
                                         boost::bind(&tcp::connection::finish, tcp_conn)));
    
    if (http_request_ptr->get_method() == http::types::REQUEST_METHOD_GET 
        || http_request_ptr->get_method() == http::types::REQUEST_METHOD_HEAD)
    {
        // the type of response we will send
        enum ResponseType {
            RESPONSE_UNDEFINED,     // initial state until we know how to respond
            RESPONSE_OK,            // normal response that includes the file's content
            RESPONSE_HEAD_OK,       // response to HEAD request (would send file's content)
            RESPONSE_NOT_FOUND,     // Not Found (404)
            RESPONSE_NOT_MODIFIED   // Not Modified (304) response to If-Modified-Since
        } response_type = RESPONSE_UNDEFINED;

        // get the If-Modified-Since request header
        const std::string if_modified_since(http_request_ptr->get_header(http::types::HEADER_IF_MODIFIED_SINCE));
        Image* dest = _storagePool.getImage(filename.c_str(), bucket.c_str());
        if (!dest) {
            sendNotFoundResponse(http_request_ptr, tcp_conn);
            return;
        }
        string lastModified = dest->getLastModifiedString();
        if (lastModified == if_modified_since) {
            response_type = RESPONSE_NOT_MODIFIED;
        } else if (http_request_ptr->get_method() == http::types::REQUEST_METHOD_HEAD) {
            response_type = RESPONSE_HEAD_OK;
        } else {
            response_type = RESPONSE_OK;
        }

        if (response_type == RESPONSE_OK) {
            // use FileSender to send a file
            FileSenderPtr sender_ptr(FileSender::create(dest,
                                                                http_request_ptr, tcp_conn,
                                                                10*1024*1024));
            sender_ptr->send();
        } else if (response_type == RESPONSE_NOT_FOUND) {
            sendNotFoundResponse(http_request_ptr, tcp_conn);
        } else {
            // sending headers only -> use our own response object
            writer->get_response().set_content_type(dest->getMimeType());

            // set Last-Modified header to enable client-side caching
            writer->get_response().add_header(http::types::HEADER_LAST_MODIFIED,
                                            dest->getLastModifiedString());

            switch(response_type) {
                case RESPONSE_UNDEFINED:
                case RESPONSE_NOT_FOUND:
                case RESPONSE_OK:
                    // this should never happen
                    BOOST_ASSERT(false);
                    break;
                case RESPONSE_NOT_MODIFIED:
                    // set "Not Modified" response
                    writer->get_response().set_status_code(http::types::RESPONSE_CODE_NOT_MODIFIED);
                    writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_NOT_MODIFIED);
                    break;
                case RESPONSE_HEAD_OK:
                    // set "OK" response (not really necessary since this is the default)
                    writer->get_response().set_status_code(http::types::RESPONSE_CODE_OK);
                    writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_OK);
                    break;
            }

            // send the response
            writer->send();
        }
    } else if (http_request_ptr->get_method() == http::types::REQUEST_METHOD_POST
               || http_request_ptr->get_method() == http::types::REQUEST_METHOD_PUT)
    {
        bool existed = _storagePool.headImage(filename.c_str(), bucket.c_str(), 0);
        if (existed && http_request_ptr->get_method() == http::types::REQUEST_METHOD_PUT) {
            sendNotFoundResponse(http_request_ptr, tcp_conn);
        } else if (!existed && http_request_ptr->get_method() == http::types::REQUEST_METHOD_DELETE) {
            sendNotFoundResponse(http_request_ptr, tcp_conn);
        } else {
            size_t requestLen = http_request_ptr->get_content_length();
            MP_ID memId = _memoryPool.alloc(requestLen);
            if (MP_NULL == memId) {
                // server internal error
                static const std::string PUT_FAILED_HTML_START =
                        "<html><head>\n"
                        "<title>500 Server Error</title>\n"
                        "</head><body>\n"
                        "<h1>Server Error</h1>\n"
                        "<p>Error writing to ";
                static const std::string PUT_FAILED_HTML_FINISH =
                        ".</p>\n"
                        "</body></html>\n";
                writer->get_response().set_status_code(http::types::RESPONSE_CODE_SERVER_ERROR);
                writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_SERVER_ERROR);
                writer->write_no_copy(PUT_FAILED_HTML_START);
                writer << algorithm::xml_encode(http_request_ptr->get_resource());
                writer->write_no_copy(PUT_FAILED_HTML_FINISH);
            } else {
                int64_t dataLen = 0;
                void* dataBuf = _memoryPool.getElementAddress(memId, dataLen);
                memcpy(dataBuf, http_request_ptr->get_content(), requestLen);
                int saveResult = _storagePool.saveImage(filename.c_str(), dataBuf, requestLen, memId, bucket.c_str());
                if (0 != saveResult) {
                    // server internal error
                    static const std::string PUT_FAILED_HTML_START =
                            "<html><head>\n"
                            "<title>500 Server Error</title>\n"
                            "</head><body>\n"
                            "<h1>Server Error</h1>\n"
                            "<p>Error writing to ";
                    static const std::string PUT_FAILED_HTML_FINISH =
                            ".</p>\n"
                            "</body></html>\n";
                    writer->get_response().set_status_code(http::types::RESPONSE_CODE_SERVER_ERROR);
                    writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_SERVER_ERROR);
                    writer->write_no_copy(PUT_FAILED_HTML_START);
                    writer << algorithm::xml_encode(http_request_ptr->get_resource());
                    writer->write_no_copy(PUT_FAILED_HTML_FINISH);
                } else {
                    static const std::string CREATED_HTML_START =
                            "<html><head>\n"
                            "<title>200 Operation Successed</title>\n"
                            "</head><body>\n"
                            "<h1>Created</h1>\n"
                            "<p>";
                    static const std::string CREATED_HTML_FINISH =
                            "</p>\n"
                            "</body></html>\n";
                    writer->get_response().set_status_code(http::types::RESPONSE_CODE_CREATED);
                    writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_CREATED);
                    writer->get_response().add_header(http::types::HEADER_LOCATION, http_request_ptr->get_resource());
                    writer->write_no_copy(CREATED_HTML_START);
                    writer << algorithm::xml_encode(http_request_ptr->get_resource());
                    writer->write_no_copy(CREATED_HTML_FINISH);
                }
            }
            writer->send();
        }
    } else if (http_request_ptr->get_method() == http::types::REQUEST_METHOD_DELETE) {
        bool result = _storagePool.deleteImage(filename.c_str(), bucket.c_str());
        if (result) {
            static const std::string CREATED_HTML_START =
                            "<html><head>\n"
                            "<title>200 Operation Successed</title>\n"
                            "</head><body>\n"
                            "<h1>Deleted</h1>\n"
                            "<p>";
            static const std::string CREATED_HTML_FINISH =
                            "</p>\n"
                            "</body></html>\n";
            writer->get_response().set_status_code(http::types::RESPONSE_CODE_CREATED);
            writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_CREATED);
            writer->get_response().add_header(http::types::HEADER_LOCATION, http_request_ptr->get_resource());
            writer->write_no_copy(CREATED_HTML_START);
            writer << algorithm::xml_encode(http_request_ptr->get_resource());
            writer->write_no_copy(CREATED_HTML_FINISH);
        } else {
            // server internal error
            static const std::string PUT_FAILED_HTML_START =
                            "<html><head>\n"
                            "<title>500 Server Error</title>\n"
                            "</head><body>\n"
                            "<h1>Server Error</h1>\n"
                            "<p>Error writing to ";
            static const std::string PUT_FAILED_HTML_FINISH =
                            ".</p>\n"
                            "</body></html>\n";
            writer->get_response().set_status_code(http::types::RESPONSE_CODE_SERVER_ERROR);
            writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_SERVER_ERROR);
            writer->write_no_copy(PUT_FAILED_HTML_START);
            writer << algorithm::xml_encode(http_request_ptr->get_resource());
            writer->write_no_copy(PUT_FAILED_HTML_FINISH);
        }
        writer->send();
    }
    // Any method not handled above is unimplemented.
    else {
        static const std::string NOT_IMPLEMENTED_HTML_START =
            "<html><head>\n"
            "<title>501 Not Implemented</title>\n"
            "</head><body>\n"
            "<h1>Not Implemented</h1>\n"
            "<p>The requested method ";
        static const std::string NOT_IMPLEMENTED_HTML_FINISH =
            " is not implemented on this server.</p>\n"
            "</body></html>\n";
        http::response_writer_ptr writer(http::response_writer::create(tcp_conn, *http_request_ptr,
                                     boost::bind(&tcp::connection::finish, tcp_conn)));
        writer->get_response().set_status_code(http::types::RESPONSE_CODE_NOT_IMPLEMENTED);
        writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_NOT_IMPLEMENTED);
        writer->write_no_copy(NOT_IMPLEMENTED_HTML_START);
        writer << algorithm::xml_encode(http_request_ptr->get_method());
        writer->write_no_copy(NOT_IMPLEMENTED_HTML_FINISH);
        writer->send();
    }
}

// FileSender member functions
FileSender::FileSender(Image*& file, const pion::http::request_ptr& http_request_ptr,
                               const pion::tcp::connection_ptr& tcp_conn,
                               unsigned long max_chunk_size)
    : m_logger(PION_GET_LOGGER("pion.FileService.FileSender")), _image(file),
    m_writer(pion::http::response_writer::create(tcp_conn, *http_request_ptr, boost::bind(&tcp::connection::finish, tcp_conn))),
    m_max_chunk_size(max_chunk_size), m_file_bytes_to_send(0), m_bytes_sent(0)
{
# if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
    PION_LOG_DEBUG(m_logger, "Preparing to send file"
                   << (_image->hasContent() ? " (cached): " : ": ")
                   << _image->getIdentifiedString());
#else
    PION_LOG_DEBUG(m_logger, "Preparing to send file"
                   << (_image->hasContent() ? " (cached): " : ": ")
                   << _image->getIdentifiedString());
#endif

        // set the Content-Type HTTP header using the file's MIME type
    m_writer->get_response().set_content_type(_image->getMimeType());

    // set Last-Modified header to enable client-side caching
    m_writer->get_response().add_header(http::types::HEADER_LAST_MODIFIED,
                                      _image->getLastModifiedString());

    // use "200 OK" HTTP response
    m_writer->get_response().set_status_code(http::types::RESPONSE_CODE_OK);
    m_writer->get_response().set_status_message(http::types::RESPONSE_MESSAGE_OK);
}

void FileSender::send(void)
{
    // check if we have nothing to send (send 0 byte response content)
    if (_image->len() <= m_bytes_sent) {
        m_writer->send();
        return;
    }

    // calculate the number of bytes to send (m_file_bytes_to_send)
    m_file_bytes_to_send = _image->len() - m_bytes_sent;
    if (m_max_chunk_size > 0 && m_file_bytes_to_send > m_max_chunk_size)
        m_file_bytes_to_send = m_max_chunk_size;

    // get the content to send (file_content_ptr)
    char *file_content_ptr;

    if (_image->hasContent()) {

        // the entire file IS cached in memory (_image.file_content)
        file_content_ptr = (char*)_image->getContentBuf() + m_bytes_sent;

    } else {
        // the file is not cached in memory

        // check if the file has been opened yet
        if (! m_file_stream.is_open()) {
            // open the file for reading
            m_file_stream.open(_image->getIdentifiedString(), std::ios::in | std::ios::binary);
            if (! m_file_stream.is_open()) {
# if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
                PION_LOG_ERROR(m_logger, "Unable to open file: "
                               << _image->getIdentifiedString());
#else
                PION_LOG_ERROR(m_logger, "Unable to open file: "
                               << _image->getIdentifiedString());
#endif
                return;
            }
        }

        // check if the content buffer was initialized yet
        if (! m_content_buf) {
            // allocate memory for the new content buffer
            m_content_buf.reset(new char[m_file_bytes_to_send]);
        }
        file_content_ptr = m_content_buf.get();

        // read a block of data from the file into the content buffer
        if (! m_file_stream.read(m_content_buf.get(), m_file_bytes_to_send)) {
            if (m_file_stream.gcount() > 0) {
# if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
                PION_LOG_ERROR(m_logger, "File size inconsistency: "
                               << _image->getIdentifiedString());
#else
                PION_LOG_ERROR(m_logger, "File size inconsistency: "
                               << _image->getIdentifiedString());
#endif
            } else {
# if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3
                PION_LOG_ERROR(m_logger, "Unable to read file: "
                               << _image->getIdentifiedString());
#else
                PION_LOG_ERROR(m_logger, "Unable to read file: "
                               << _image->getIdentifiedString());
#endif
            }
            return;
        }
    }

    // send the content
    m_writer->write_no_copy(file_content_ptr, m_file_bytes_to_send);

    if (m_bytes_sent + m_file_bytes_to_send >= _image->len()) {
        // this is the last piece of data to send
        if (m_bytes_sent > 0) {
            // send last chunk in a series
            m_writer->send_final_chunk(boost::bind(&FileSender::handle_write,
                                                 shared_from_this(),
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::bytes_transferred));
        } else {
            // sending entire file at once
            m_writer->send(boost::bind(&FileSender::handle_write,
                                       shared_from_this(),
                                       boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred));
        }
    } else {
        // there will be more data -> send a chunk
        m_writer->send_chunk(boost::bind(&FileSender::handle_write,
                                        shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
    }
}

void FileSender::handle_write(const boost::system::error_code& write_error,
                                 std::size_t /* bytes_written */)
{
    bool finished_sending = true;

    if (write_error) {
        // encountered error sending response data
        m_writer->get_connection()->set_lifecycle(tcp::connection::LIFECYCLE_CLOSE); // make sure it will get closed
        PION_LOG_WARN(m_logger, "Error sending file (" << write_error.message() << ')');
    } else {
        // response data sent OK

        // use m_file_bytes_to_send instead of bytes_written; bytes_written
        // includes bytes for HTTP headers and chunking headers
        m_bytes_sent += m_file_bytes_to_send;

        if (m_bytes_sent >= _image->len()) {
            // finished sending
            PION_LOG_DEBUG(m_logger, "Sent "
                           << (m_file_bytes_to_send < _image->len() ? "file chunk" : "complete file")
                           << " of " << m_file_bytes_to_send << " bytes (finished"
                           << (m_writer->get_connection()->get_keep_alive() ? ", keeping alive)" : ", closing)") );
        } else {
            // NOT finished sending
            PION_LOG_DEBUG(m_logger, "Sent file chunk of " << m_file_bytes_to_send << " bytes");
            finished_sending = false;
            m_writer->clear();
        }
    }

    if (finished_sending) {
        // connection::finish() calls tcp::server::finish_connection, which will either:
        // a) call http::server::handle_connection again if keep-alive is true; or,
        // b) close the socket and remove it from the server's connection pool
        m_writer->get_connection()->finish();
    } else {
        send();
    }
}
