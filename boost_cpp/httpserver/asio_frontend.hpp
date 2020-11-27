#ifndef ASIO_FRONTEND_H
#define ASIO_FRONTEND_H

#include <memory>
#include <string>
#include <tuple>

/*
#define port 8888
#define ip "::"
#define ssl False
#define max_connection_backlog 1024
*/


class FrontendConfig{
public:
    int port{8888};
    bool ssl{false};
    int max_connection_backlog{1024};
    bool tcp_nodelay{true};
    int thread_pool_size{20};
    bool daemon{false};
};


class Frontend {
  class Impl;
  std::unique_ptr<Impl> impl;
public:
  Frontend(FrontendConfig*);
  ~Frontend();

  int init();
  int run();
  void stop();
  void join();
};

#endif
