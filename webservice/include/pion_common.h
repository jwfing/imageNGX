#ifndef CN_LEANCLOUD_IMAGESSERVICE_PION_COMMON_INCLUDE_H_
#define CN_LEANCLOUD_IMAGESSERVICE_PION_COMMON_INCLUDE_H_

#include <pion/http/plugin_service.hpp>
#include <pion/http/response_writer.hpp>
#include <pion/algorithm.hpp>
#include <pion/http/response_writer.hpp>
#include <pion/user.hpp>
#include <pion/http/server.hpp>

using namespace pion;
using namespace pion::http;

typedef pion::http::server HTTPServer;
typedef pion::http::request_ptr HTTPRequestPtr;
typedef pion::tcp::connection_ptr TCPConnectionPtr;
typedef pion::tcp::connection TCPConnection;
typedef pion::http::response_writer_ptr HTTPResponseWriterPtr;
typedef pion::http::response_writer HTTPResponseWriter;
typedef pion::http::types HTTPTypes;

#endif
