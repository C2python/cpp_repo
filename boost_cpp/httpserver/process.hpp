#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <boost/beast/http/verb.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <iostream>
#include <string>
#include "asio_client.hpp"

//#define buf_size 1000

using verb = boost::beast::http::verb;

void process(verb, ClientIO* client, boost::asio::yield_context yield,int& http_ret);

#endif