#ifndef ASIO_CLIENT_H
#define ASIO_CLIENT_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace beast = boost::beast;
using parser_type = beast::http::request_parser<beast::http::buffer_body>;

#endif