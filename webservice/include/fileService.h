// ---------------------------------------------------------------------
// pion:  a Boost C++ framework for building lightweight HTTP interfaces
// ---------------------------------------------------------------------
// Copyright (C) 2007-2014 Splunk Inc.  (https://github.com/splunk/pion)
//
// Distributed under the Boost Software License, Version 1.0.
// See http://www.boost.org/LICENSE_1_0.txt
//

#ifndef __PION_FILESERVICE_HEADER__
#define __PION_FILESERVICE_HEADER__

#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/thread/once.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_array.hpp>
#include <pion/config.hpp>
#include <pion/logger.hpp>
#include <pion/hash_map.hpp>
#include <pion/http/plugin_service.hpp>
#include <pion/http/request.hpp>
#include <pion/http/response_writer.hpp>
#include <pion/http/server.hpp>
#include <string>
#include <map>

using pion::logger;

#include "commonService.h"

///
/// FileSender: class used to send files to clients using HTTP responses
/// 
class FileSender : 
    public boost::enable_shared_from_this<FileSender>,
    private boost::noncopyable
{
public:
    /**
     * creates new FileSender objects
     *
     * @param file disk file object that should be sent
     * @param http_request_ptr HTTP request that we are responding to
     * @param tcp_conn TCP connection used to send the file
     * @param max_chunk_size sets the maximum chunk size (default=0, unlimited)
     */
    static inline boost::shared_ptr<FileSender>
        create(Image*& file,
               const pion::http::request_ptr& http_request_ptr,
               const pion::tcp::connection_ptr& tcp_conn,
               unsigned long max_chunk_size = 0) 
    {
        return boost::shared_ptr<FileSender>(new FileSender(file, http_request_ptr,
                                                                    tcp_conn, max_chunk_size));
    }

    /// default virtual destructor 
    virtual ~FileSender() {}

    /// Begins sending the file to the client.  Following a call to this
    /// function, it is not thread safe to use your reference to the
    /// FileSender object.
    void send(void);

    /// sets the logger to be used
    inline void set_logger(logger log_ptr) { m_logger = log_ptr; }

    /// returns the logger currently in use
    inline logger get_logger(void) { return m_logger; }


protected:

    /**
     * protected constructor restricts creation of objects (use create())
     * 
     * @param file disk file object that should be sent
     * @param http_request_ptr HTTP request that we are responding to
     * @param tcp_conn TCP connection used to send the file
     * @param max_chunk_size sets the maximum chunk size
     */
    FileSender(Image*& file,
                   const pion::http::request_ptr& http_request_ptr,
                   const pion::tcp::connection_ptr& tcp_conn,
                   unsigned long max_chunk_size);

    /**
     * handler called after a send operation has completed
     *
     * @param write_error error status from the last write operation
     * @param bytes_written number of bytes sent by the last write operation
     */
    void handle_write(const boost::system::error_code& write_error,
                     std::size_t bytes_written);


    /// primary logging interface used by this class
    logger                              m_logger;


private:

    /// the disk file we are sending
    Image*                                _image;

    /// the HTTP response we are sending
    pion::http::response_writer_ptr        m_writer;

    /// used to read the file from disk if it is not already cached in memory
    boost::filesystem::ifstream             m_file_stream;

    /// buffer used to send file content
    boost::shared_array<char>               m_content_buf;

    /**
     * maximum chunk size (in bytes): files larger than this size will be
     * delivered to clients using HTTP chunked responses.  A value of
     * zero means that the size is unlimited (chunking is disabled).
     */
    int m_max_chunk_size;

    /// the number of file bytes send in the last operation
    int m_file_bytes_to_send;

    /// the number of bytes we have sent so far
    int m_bytes_sent;
};

/// data type for a FileSender pointer
typedef boost::shared_ptr<FileSender>       FileSenderPtr;


///
/// FileService: web service that serves regular files
/// 
class FileService : public CommonService
{
public:

    // default constructor and destructor
    FileService(MemoryPool& memoryPool, StoragePool& storagePool);
    virtual ~FileService() {}

    /// handles requests for FileService
    virtual void operator()(const pion::http::request_ptr& http_request_ptr,
                            const pion::tcp::connection_ptr& tcp_conn);

protected:
    /// primary logging interface used by this class
    logger                  m_logger;

};

#endif
