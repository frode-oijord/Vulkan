
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/signal_set.hpp>

namespace net = boost::asio;                    // namespace asio
using tcp = net::ip::tcp;                       // from <boost/asio/ip/tcp.hpp>
using error_code = boost::system::error_code;   // from <boost/system/error_code.hpp>

namespace beast = boost::beast;
namespace http = boost::beast::http;            // from <boost/beast/http.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

#define THROW_ON_ERROR(__function__)          \
{                                             \
  error_code ec;                              \
  __function__;                               \
  if (ec) {                                   \
    throw std::runtime_error(ec.message());   \
  }                                           \
}                                             \

#define RETURN_ON_ERROR(__ec__)                                                         \
if (__ec__) {                                                                           \
  if (__ec__ != net::error::operation_aborted && __ec__ != websocket::error::closed) {  \
    std::cerr << __ec__.message() << "\n";                                              \
  }                                                                                     \
  return;                                                                               \
}                                                                                       \


class websocket_session;

// Represents the shared server state
class shared_state {
public:
  void join(websocket_session* session)
  {
    this->sessions.insert(session);
  }

  void leave(websocket_session* session)
  {
    this->sessions.erase(session);
  }

  void send(std::string message);

private:
  std::unordered_set<websocket_session*> sessions;
};


class websocket_session : public std::enable_shared_from_this<websocket_session> {
public:
  websocket_session(tcp::socket socket, shared_state* state) :
    socket(std::move(socket)),
    state(state)
  {}

  ~websocket_session()
  {
    this->state->leave(this);
  }

  void initiate_accept(std::shared_ptr<http::request<http::string_body>> request)
  {
    auto buffer = std::make_shared<beast::flat_buffer>();
    this->socket.async_accept(*request, [self = shared_from_this(), buffer](error_code ec) {
      RETURN_ON_ERROR(ec);
      self->state->join(self.get());
      self->initiate_read(std::move(buffer));
    });
  }

  // Send a message
  void send(std::shared_ptr<std::string> ss)
  {
    // Always add to queue
    this->queue.push_back(std::move(ss));

    if (this->queue.size() <= 1) {
      // We are not currently writing, so send this immediately
      this->initiate_write();
    }
  }

private:
  void initiate_read(std::shared_ptr<beast::flat_buffer> buffer)
  {
    this->socket.async_read(*buffer, [self = shared_from_this(), buffer](error_code ec, std::size_t bytes) {
      RETURN_ON_ERROR(ec);
      // Send to all connections
      self->state->send(beast::buffers_to_string(buffer->data()));
      // Clear the buffer
      buffer->consume(buffer->size());
      // Read another message
      self->initiate_read(std::move(buffer));
    });
  }

  void initiate_write()
  {
    this->socket.async_write(net::buffer(*this->queue.front()), [self = shared_from_this()](error_code ec, std::size_t bytes) {
      RETURN_ON_ERROR(ec);
      self->queue.erase(self->queue.begin());
      if (!self->queue.empty()) {
        self->initiate_write();
      }
    });
  }

  shared_state* state;
  websocket::stream<tcp::socket> socket;
  std::vector<std::shared_ptr<std::string const>> queue;
};


void shared_state::send(std::string message)
{
  auto ss = std::make_shared<std::string>(std::move(message));

  for (auto session : this->sessions) {
    session->send(ss);
  }
}


class http_session : public std::enable_shared_from_this<http_session> {
public:
  http_session(tcp::socket socket, shared_state* state) :
    socket(std::move(socket)),
    state(state)
  {}

  void initiate_read(std::shared_ptr<beast::flat_buffer> buffer)
  {
    auto request = std::make_shared<http::request<http::string_body>>();
    http::async_read(this->socket, *buffer, *request, [self = shared_from_this(), request, buffer](error_code ec, std::size_t bytes) {
      self->on_read(request, buffer, bytes, ec);
    });
  }

private:
  template <class Response>
  void initiate_write(std::shared_ptr<Response> response, std::shared_ptr<beast::flat_buffer> buffer)
  {
    response->set(http::field::server, BOOST_BEAST_VERSION_STRING);
    response->set(http::field::content_type, "text/html");
    response->prepare_payload();

    http::async_write(this->socket, *response, [self = shared_from_this(), response, buffer](error_code ec, std::size_t /* bytes */)
    {
      if (response->need_eof()) {
        self->socket.shutdown(tcp::socket::shutdown_send, ec);
      }
      else {
        RETURN_ON_ERROR(ec);
        self->initiate_read(std::move(buffer));
      }
    });
  }

  void on_read(std::shared_ptr<http::request<http::string_body>> request, 
               std::shared_ptr<beast::flat_buffer> buffer, std::size_t, error_code ec)
  {
    // This means they closed the connection
    if (ec == http::error::end_of_stream) {
      this->socket.shutdown(tcp::socket::shutdown_send, ec);
      return;
    }

    RETURN_ON_ERROR(ec);

    // See if it is a WebSocket Upgrade
    if (websocket::is_upgrade(*request)) {
      // Create a WebSocket session by transferring the socket
      std::make_shared<websocket_session>(std::move(this->socket), this->state)->initiate_accept(request);
      return;
    }

    // Make sure we can handle the method
    if (request->method() != http::verb::get) {
      auto response = std::make_shared<http::response<http::string_body>>(http::status::bad_request, request->version());
      response->body() = "Unknown HTTP method";
      this->initiate_write(std::move(response), std::move(buffer));
    }

    http::file_body::value_type body;
    body.open("./index.html", beast::file_mode::scan, ec);

    if (ec) {
      auto response = std::make_shared<http::response<http::string_body>>(http::status::internal_server_error, request->version());
      response->body() = ec.message();
      this->initiate_write(std::move(response), std::move(buffer));
    }

    // Respond to GET request
    auto response = std::make_shared<http::response<http::file_body>>(http::status::ok, request->version());
    response->body() = std::move(body);
    this->initiate_write(std::move(response), std::move(buffer));
  }

  tcp::socket socket;
  shared_state* state;
};

void accept_connection(tcp::acceptor* acceptor, tcp::socket & socket, shared_state* state)
{
  acceptor->async_accept(socket, [acceptor, &socket, state](error_code ec) {
    RETURN_ON_ERROR(ec);
    auto buffer = std::make_shared<beast::flat_buffer>();
    std::make_shared<http_session>(std::move(socket), state)->initiate_read(std::move(buffer));
    accept_connection(acceptor, socket, state);
  });
}

int main(int argc, char* argv[])
{
  tcp::endpoint endpoint{ net::ip::make_address("0.0.0.0"), 8080 };

  net::io_context ioc;
  tcp::socket socket(ioc);
  tcp::acceptor acceptor(ioc);

  THROW_ON_ERROR(acceptor.open(endpoint.protocol(), ec));
  THROW_ON_ERROR(acceptor.bind(endpoint, ec));
  THROW_ON_ERROR(acceptor.listen(net::socket_base::max_listen_connections, ec));
  acceptor.set_option(net::socket_base::reuse_address(true));

  shared_state state;
  accept_connection(&acceptor, socket, &state);

  // Capture SIGINT and SIGTERM to perform a clean shutdown
  net::signal_set signals(ioc, SIGINT, SIGTERM);
  signals.async_wait([&ioc](error_code const&, int) {
    ioc.stop();
  });

  ioc.run();

  return EXIT_SUCCESS;
}
