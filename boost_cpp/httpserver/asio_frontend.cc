#include <atomic>
#include <thread>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <cerrno>

#include <unistd.h>
#include <sys/socket.h>

#include <boost/asio.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/context/protected_fixedsize_stack.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>


#include "asio_frontend.hpp"
#include "asio_client.hpp"
#include "client.hpp"
#include "client_io_filter.hpp"
#include "process.hpp"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

using parse_buffer = boost::beast::flat_static_buffer<65536>;

/*
** Request ID
*/
std::atomic<int64_t> max_req_id = { 0 };

/*
** use mmap/mprotect to allocate 512k coroutine stacks
*/
auto make_stack_allocator() {
  return boost::context::protected_fixedsize_stack{512*1024};
}


/*
** ASIO Stream IO
*/

template <typename Stream>
class StreamIO : public ClientIO {
  Stream& stream;
  boost::asio::yield_context yield;
  parse_buffer& buffer;
 public:
  StreamIO(Stream& stream, parser_type& parser,
           boost::asio::yield_context yield,
           parse_buffer& buffer, bool is_ssl,
           const tcp::endpoint& local_endpoint,
           const tcp::endpoint& remote_endpoint)
      : ClientIO(parser, is_ssl, local_endpoint, remote_endpoint),
         stream(stream), yield(yield), buffer(buffer)
  {}

  size_t write_data(const char* buf, size_t len) override {
    boost::system::error_code ec;
    auto bytes = boost::asio::async_write(stream, boost::asio::buffer(buf, len),
                                          yield[ec]);
    if (ec) {
      std::cout << "write_data failed: " << ec.message() << std::endl;
      if (ec==boost::asio::error::broken_pipe) {
        boost::system::error_code ec_ignored;
        stream.lowest_layer().shutdown(tcp::socket::shutdown_both, ec_ignored);
      }
      throw Exception(ec.value(), std::system_category());
    }
    return bytes;
  }

  size_t recv_body(char* buf, size_t max) override {
    auto& message = parser.get();
    auto& body_remaining = message.body();
    body_remaining.data = buf;
    body_remaining.size = max;

    while (body_remaining.size && !parser.is_done()) {
      boost::system::error_code ec;
      http::async_read_some(stream, buffer, parser, yield[ec]);
      if (ec == http::error::need_buffer) {
        break;
      }
      if (ec) {
        std::cout << "failed to read body: " << ec.message() << std::endl;
        throw Exception(ec.value(), std::system_category());
      }
    }
    return max - body_remaining.size;
  }
};

/*
** output the http version as a string, ie 'HTTP/1.1'
*/
struct http_version {
  unsigned major_ver;
  unsigned minor_ver;
  explicit http_version(unsigned version)
    : major_ver(version / 10), minor_ver(version % 10) {}
};
std::ostream& operator<<(std::ostream& out, const http_version& v) {
  return out << "HTTP/" << v.major_ver << '.' << v.minor_ver;
}


/* 
** log an http header value or '-' if it's missing
*/
struct log_header {
  const http::fields& fields;
  http::field field;
  std::string_view quote;
  log_header(const http::fields& fields, http::field field,
             std::string_view quote = "")
    : fields(fields), field(field), quote(quote) {}
};
std::ostream& operator<<(std::ostream& out, const log_header& h) {
  auto p = h.fields.find(h.field);
  if (p == h.fields.end()) {
    return out << '-';
  }
  return out << h.quote << p->value() << h.quote;
}

template <typename Stream>
void handle_connection(boost::asio::io_context& context,
                       Stream& stream,
                       parse_buffer& buffer, 
                       boost::system::error_code& ec,
                       boost::asio::yield_context yield)
{
  std::cout<<"Accept New Connection"<<std::endl;
  int64_t req = max_req_id++;
  // limit header to 4k, since we read it all into a single flat_buffer
  static constexpr size_t header_limit = 4096;
  // don't impose a limit on the body, since we read it in pieces
  static constexpr size_t body_limit = std::numeric_limits<size_t>::max();

    // read messages from the stream until eof
  for (;;) {
    // configure the parser
    parser_type parser;
    parser.header_limit(header_limit);
    parser.body_limit(body_limit);

    // parse the header
    http::async_read_header(stream, buffer, parser, yield[ec]);
    if (ec == boost::asio::error::connection_reset ||
        ec == boost::asio::error::bad_descriptor ||
        ec == boost::asio::error::operation_aborted ||
        ec == http::error::end_of_stream) {
      std::cout << "failed to read header: " << ec.message() << std::endl;
      return;
    }
    auto& message = parser.get();
    for (auto header = message.begin(); header != message.end(); ++header) {
      std::cout<<header->name_string()<<": "<<header->value().to_string()<<std::endl;
    }
    auto& body = parser.get().body();
    if (ec) {
      std::cout << "failed to read header: " << ec.message() << std::endl;
      http::response<http::empty_body> response;
      response.result(http::status::bad_request);
      response.version(message.version() == 10 ? 10 : 11);
      response.prepare_payload();
      http::async_write(stream, response, yield[ec]);
      if (ec) {
        std::cout << "failed to write response: " << ec.message() << std::endl;
      }
      std::cout << "====== req done http_status=400 ======" << std::endl;
      return;
    }
    auto& socket = stream.lowest_layer();
    const auto& remote_endpoint = socket.remote_endpoint(ec);
    std::cout << "beast: " << std::hex << &req << std::dec << ": "
            << remote_endpoint.address() <<' '
            << message.method_string() << ' ' << message.target() << ' '
            << http_version{message.version()} << "\" " << ' '
            <<log_header{message, http::field::referer, "\""} << ' '
            << log_header{message, http::field::user_agent, "\""} << ' '
            << log_header{message, http::field::range} <<' '<<"keepalive: "
            << message[beast::http::field::keep_alive]<< std::endl;
    bool is_ssl = false;
    StreamIO clientio{stream, parser, yield, buffer, is_ssl,
                      socket.local_endpoint(),
                      remote_endpoint};
    clientio.init_env();
    auto real_client_io = add_reordering(
                            add_chunking(
                              add_conlen_controlling(
                                &clientio)));
    int http_ret = 0;
    process(message.method(),&clientio,yield,http_ret);
    if (http_ret < 0){
      return;
    }
    if (!parser.keep_alive()) {
      return;
    }else{
      std::cout<<"Keep-alive"<<std::endl;
    }
    
    while (!parser.is_done()) {
      static std::array<char, 1024> discard_buffer;

      auto& body = parser.get().body();
      body.size = discard_buffer.size();
      body.data = discard_buffer.data();

      http::async_read_some(stream, buffer, parser, yield[ec]);
      if (ec == http::error::need_buffer) {
        continue;
      }
      if (ec == boost::asio::error::connection_reset) {
        return;
      }
      if (ec) {
        std::cout << "failed to discard unread message: "<< ec.message() << std::endl;
        return;
      }
    }
  }
}

struct Connection : boost::intrusive::list_base_hook<> {
  tcp::socket& socket;
  Connection(tcp::socket& socket) : socket(socket) {}
};


class ConnectionList {
  using List = boost::intrusive::list<Connection>;
  List connections;
  std::mutex mutex;

  void remove(Connection& c) {
    std::lock_guard lock{mutex};
    if (c.is_linked()) {
      connections.erase(List::s_iterator_to(c));
    }
  }
 public:
  class Guard {
    ConnectionList *list;
    Connection *conn;
   public:
    Guard(ConnectionList *list, Connection *conn) : list(list), conn(conn) {}
    ~Guard() { list->remove(*conn); }
  };
  [[nodiscard]] Guard add(Connection& conn) {
    std::lock_guard lock{mutex};
    connections.push_back(conn);
    return Guard{this, &conn};
  }
  void close(boost::system::error_code& ec) {
    std::lock_guard<std::mutex> lock{mutex};
    for (auto& conn : connections) {
      conn.socket.close(ec);
    }
    connections.clear();
  }
};

class AsioFrontend{
    FrontendConfig* conf;
    boost::asio::io_context context;
    struct Listener {
        tcp::endpoint endpoint;
        tcp::acceptor acceptor;
        tcp::socket socket;
        bool use_ssl = false;
        bool use_nodelay = false;

        explicit Listener(boost::asio::io_context& context)
        : acceptor(context), socket(context) {}
    };
    std::vector<Listener> listeners;
    ConnectionList connections;

    /*
    ** work guard keep run() threads busy
    */
    using Executor = boost::asio::io_context::executor_type;
    std::optional<boost::asio::executor_work_guard<Executor>> work;

    std::vector<std::thread> threads;
    std::atomic<bool> going_down{false};

    void accept(Listener& listener, boost::system::error_code ec);
public:
    AsioFrontend(FrontendConfig* conf):conf(conf){}
    int init();
    int run();
    void stop();
    void join();
};

unsigned short parse_port(const char *input, boost::system::error_code& ec)
{
  char *end = nullptr;
  auto port = std::strtoul(input, &end, 10);
  if (port > std::numeric_limits<unsigned short>::max()) {
    ec.assign(ERANGE, boost::system::system_category());
  } else if (port == 0 && end == input) {
    ec.assign(EINVAL, boost::system::system_category());
  }
  return port;
}

int AsioFrontend::init(){
    boost::system::error_code ec;
    auto port = conf->port;
    
    listeners.emplace_back(context);
    listeners.back().endpoint.port(port);

    listeners.emplace_back(context);
    listeners.back().endpoint = tcp::endpoint(tcp::v6(), port);
    auto nodelay = conf->tcp_nodelay;
    if (nodelay) {
        for (auto& l : listeners) {
            l.use_nodelay = nodelay;
        }
    }
    bool socket_bound = false;
    for (auto& l:listeners){
        l.acceptor.open(l.endpoint.protocol(), ec);
        if (ec) {
            if (ec == boost::asio::error::address_family_not_supported) {
                continue;
            }

            return -ec.value();
        }
        if (l.endpoint.protocol() == tcp::v6()) {
            l.acceptor.set_option(boost::asio::ip::v6_only(true), ec);
            if (ec) {
                std::cout << "failed to set v6_only socket option: "
                    << ec.message() << std::endl;
                return -ec.value();
            }
        }
        l.acceptor.set_option(tcp::acceptor::reuse_address(true));
        l.acceptor.bind(l.endpoint, ec);
        if (ec) {
            std::cout << "failed to bind address " << l.endpoint
                << ": " << ec.message() << std::endl;
            return -ec.value();
        }
        auto max_connection_backlog = boost::asio::socket_base::max_listen_connections;
        if (max_connection_backlog > conf->max_connection_backlog){
            max_connection_backlog = conf->max_connection_backlog;
        }
        l.acceptor.listen(max_connection_backlog);
        l.acceptor.async_accept(l.socket,
                                [this, &l] (boost::system::error_code ec) {
                                accept(l, ec);
                                });

        std::cout << "frontend listening on " << l.endpoint << std::endl;
        socket_bound = true;
    }
    if (!socket_bound) {
        std::cout << "Unable to listen at any endpoints" << std::endl;
        return -EINVAL;
    }
        
    return 1;
}

void AsioFrontend::accept(Listener& l, boost::system::error_code ec){
    if (!l.acceptor.is_open()) {
        return;
    } else if (ec == boost::asio::error::operation_aborted) {
        return;
    } else if (ec) {
        std::cout << "accept failed: " << ec.message() << std::endl;
        return;
    }
    auto socket = std::move(l.socket);
    tcp::no_delay options(l.use_nodelay);
    socket.set_option(options,ec);
    l.acceptor.async_accept(l.socket,
                        [this, &l] (boost::system::error_code ec) {
                        accept(l, ec);
                        });
    boost::asio::spawn(context,
      [this, s=std::move(socket)] (boost::asio::yield_context yield) mutable {
        Connection conn{s};
        auto c = connections.add(conn);
        auto buffer = std::make_unique<parse_buffer>();
        boost::system::error_code ec;
        handle_connection(context, s, *buffer, ec, yield);
        s.shutdown(tcp::socket::shutdown_both, ec);
      }/*, make_stack_allocator()*/);
}

int AsioFrontend::run(){
    int thread_count = conf->thread_pool_size;
    threads.reserve(thread_count);

    work.emplace(boost::asio::make_work_guard(context));

    for (int i=0;i<thread_count;++i){
        threads.emplace_back([=]{

            // request warnings on synchronous librados calls in this thread
            //is_asio_thread = true;
            boost::system::error_code ec;
            context.run(ec);
        });
    }
    /*
    boost::system::error_code ec;
    context.run(ec);
    */
    return 0;
}

void AsioFrontend::stop(){
    going_down = true;

    boost::system::error_code ec;
    // close all listeners
    for (auto& listener : listeners) {
        listener.acceptor.close(ec);
    }
    // close all connections
    connections.close(ec);
}

void AsioFrontend::join()
{
  if (!going_down) {
    stop();
  }
  work.reset();

  std::cout << "frontend joining threads..." << std::endl;
  for (auto& thread : threads) {
    thread.join();
  }
  std::cout<< "frontend done" << std::endl;
}

class Frontend::Impl : public AsioFrontend {
 public:
  Impl(FrontendConfig* conf): AsioFrontend(conf) {}
};

Frontend::Frontend(FrontendConfig* conf):impl(new Impl(conf))
{
}

Frontend::~Frontend() = default;

int Frontend::init()
{
  return impl->init();
}

int Frontend::run()
{
  return impl->run();
}

void Frontend::stop()
{
  impl->stop();
}

void Frontend::join()
{
  impl->join();
}


static int signal_fd[2] = {0, 0};


void signal_shutdown()
{
  int val = 0;
  int ret = write(signal_fd[0], (char *)&val, sizeof(val));
  if (ret < 0) {
    std::cout << "ERROR: " << __func__ << ": write() returned "
         << errno << std::endl;
  }
}

static void wait_shutdown()
{
  int val;
  int r = read(signal_fd[1], &val, sizeof(val));
  if (r < 0) {
    std::cout << "safe_read_exact returned with error" << std::endl;
  }
}

static int signal_fd_init()
{
  return socketpair(AF_UNIX, SOCK_STREAM, 0, signal_fd);
}

static void signal_fd_finalize()
{
  close(signal_fd[0]);
  close(signal_fd[1]);
}

static void handle_sigterm(int signum, siginfo_t *info, void *data) {
  std::cout<< __func__ << std::endl;
  signal_shutdown();

}

void register_signal(int signum){
    struct sigaction action;
    bzero(&action, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO | SA_RESETHAND;
    action.sa_sigaction = &handle_sigterm;
    sigaction(signum, &action, NULL);
}

int initialize(){
    int ret = signal_fd_init();
    register_signal(SIGTERM);
    register_signal(SIGINT);
    register_signal(SIGUSR1);
    return ret;
}


int main(int argc,char* argv[]){
    int ret = initialize();
    if (ret < 0){
      return -1;
    }
    FrontendConfig* conf = new FrontendConfig();
    Frontend* fe= new Frontend(conf);
    if (conf->daemon){
      int ret = daemon(1,1);
      if (ret < 0)
        return EXIT_FAILURE;
    }
    fe->init();
    std::cout<<"Before Run"<<std::endl;
    fe->run();
    std::cout<<"After Run"<<std::endl;
    wait_shutdown();
    fe->stop();
    fe->join();
    delete conf;
    delete fe;
    signal_fd_finalize();
    return EXIT_SUCCESS;
}