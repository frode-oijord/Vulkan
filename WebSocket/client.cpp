#include <Scheme/Scheme.h>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/signal_set.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

#define RETURN_ON_ERROR(__ec__)                                                         \
if (__ec__) {                                                                           \
  std::cerr << __ec__.message() << "\n";                                                \
  return;                                                                               \
}                                                                                       \

// Sends a WebSocket message and prints the response
class connection {
  tcp::resolver resolver;
  const std::string host;
  const std::string port;
  websocket::stream<beast::tcp_stream> stream;

public:
  // Resolver and socket require an io_context
  explicit connection(net::io_context& ioc, const std::string& host, const std::string& port) :
    resolver(net::make_strand(ioc)),
    host(host),
    port(port),
    stream(net::make_strand(ioc))
  {
    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(this->stream).expires_never();

    // Set suggested timeout settings for the websocket
    this->stream.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

    // Set a decorator to change the User-Agent of the handshake
    this->stream.set_option(websocket::stream_base::decorator([](websocket::request_type& req) {
      req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
      }));
  }

  // Start the asynchronous operation
  void on_open(std::function<void()> on_open_cb)
  {
    // Look up the domain name
    this->resolver.async_resolve(host, port, [=](beast::error_code ec, tcp::resolver::results_type results) {
      RETURN_ON_ERROR(ec);

      // Set the timeout for the operation
      beast::get_lowest_layer(this->stream).expires_after(std::chrono::seconds(30));

      // Make the connection on the IP address we get from a lookup
      beast::get_lowest_layer(this->stream).async_connect(results, [=](beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
        RETURN_ON_ERROR(ec);

        // Perform the websocket handshake
        this->stream.async_handshake(this->host, "/", [=](beast::error_code ec) {
          RETURN_ON_ERROR(ec);
          on_open_cb();
          });
        });
      });
  }

  void write(const std::string message)
  {
    this->stream.write(net::buffer(message));
  }

  void write_async(const std::string message)
  {
    this->stream.async_write(net::buffer(message), [this](beast::error_code ec, std::size_t) {
      RETURN_ON_ERROR(ec);
    });
  }


  std::string read()
  {
    beast::flat_buffer buffer;
    this->stream.read(buffer);
    return beast::buffers_to_string(buffer.data());
  }

  void read_async()
  {
    beast::flat_buffer buffer;
    this->stream.async_read(buffer, [this, buffer](beast::error_code ec, std::size_t) {
      RETURN_ON_ERROR(ec);
      std::cout << beast::buffers_to_string(buffer.data()) << std::endl;
      this->read_async();
      });
  }

  void close()
  {
    this->stream.async_close(websocket::close_code::normal, [](beast::error_code ec) {
      RETURN_ON_ERROR(ec);
      std::cout << "connection closed." << std::endl;
      });
  }
};


scm::fun_ptr print = [](const scm::List& lst) {
  std::string s = std::any_cast<std::string>(lst[0]);
  std::cout << s << std::endl;
  return nullptr;
};


int main(int argc, char** argv)
{
  scm::env_ptr env = scm::global_env();
  scm::env_ptr app_env = std::make_shared<scm::Env>();
  app_env->inner.insert({ "print", print });

  env->outer = app_env;

  net::io_context ioc;
  connection ws(ioc, "localhost", "8080");

  ws.on_open([&]() {
    std::cout << "WebSocket Scheme REPL" << std::endl;
    while (true) {
      std::cout << "> ";
      std::string input;
      std::getline(std::cin, input);
      ws.write(input);

      std::string reply = ws.read();
      std::any exp = scm::read(reply.begin(), reply.end());
      exp = scm::eval(exp, env);
      // scm::print(exp, std::cout); std::cout << std::endl;
    }
    });

  // Capture SIGINT and SIGTERM to perform a clean shutdown
  net::signal_set signals(ioc, SIGINT, SIGTERM);
  signals.async_wait([&ioc, &ws](boost::system::error_code const&, int) {
    ioc.stop();
    });


  ioc.run();

  return EXIT_SUCCESS;
}