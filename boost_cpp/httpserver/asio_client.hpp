#ifndef ASIO_CLIENT_H
#define ASIO_CLIENT_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <map>
#include <string>
#include <iostream>
#include <time.h>
#include <cstdio>
#include <optional>

#include "client.hpp"

namespace beast = boost::beast;
using parser_type = beast::http::request_parser<beast::http::buffer_body>;

class ClientIO : public RestfulClient,
                 public BuffererSink {
 protected:
  parser_type& parser;
 private:
  const bool is_ssl;
  using endpoint_type = boost::asio::ip::tcp::endpoint;
  endpoint_type local_endpoint;
  endpoint_type remote_endpoint;

  std::map<std::string, std::string> env_map;
  
  StaticOutputBufferer<> txbuf;

 public:
  ClientIO(parser_type& parser, bool is_ssl,
           const endpoint_type& local_endpoint,
           const endpoint_type& remote_endpoint);
  ~ClientIO() override;

  int init_env();
  size_t complete_request() override;
  void flush() override;
  size_t send_status(int status, const char *status_name) override;
  size_t send_100_continue() override;
  size_t send_header(const std::string_view& name,
                     const std::string_view& value) override;
  size_t send_content_length(uint64_t len) override;
  size_t complete_header() override;

  size_t send_body(const char* buf, size_t len) override {
    return write_data(buf, len);
  }
  std::optional<std::string> get_val(const std::string& name){
    if (env_map.find(name) != env_map.end()){
      return env_map[name];
    } 
    return std::nullopt;
  }
};

#endif