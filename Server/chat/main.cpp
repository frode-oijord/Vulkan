
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>

#include <memory>
#include <string>
#include <iostream>
#include <unordered_set>

namespace bpo = boost::program_options;

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
	websocket_session(tcp::socket socket, std::shared_ptr<shared_state> state) :
		socket(std::move(socket)),
		state(std::move(state))
	{}

	~websocket_session()
	{
		this->state->leave(this);
	}

	void initiate_accept(std::shared_ptr<http::request<http::string_body>> request)
	{
		std::cout << "Accepting websocket upgrade..." << std::endl;
		this->socket.async_accept(*request, [self = shared_from_this()](error_code ec) {
			RETURN_ON_ERROR(ec);
			self->state->join(self.get());
			self->initiate_read();
		});
	}

	void send(std::shared_ptr<std::string> ss)
	{
		this->queue.push_back(std::move(ss));
		if (this->queue.size() <= 1) {
			this->initiate_write();
		}
	}

private:
	void initiate_read()
	{
		this->socket.async_read(this->buffer, [self = shared_from_this()](error_code ec, std::size_t bytes) {
			RETURN_ON_ERROR(ec);
			self->state->send(beast::buffers_to_string(self->buffer.data()));
			self->buffer.consume(self->buffer.size());
			std::cout << "Read from websocket..." << std::endl;
			self->initiate_read();
		});
	}

	void initiate_write()
	{
		this->socket.async_write(net::buffer(*this->queue.front()), [self = shared_from_this()](error_code ec, std::size_t bytes) {
			RETURN_ON_ERROR(ec);
			self->queue.erase(self->queue.begin());
			std::cout << "Wrote to websocket..." << std::endl;
			if (!self->queue.empty()) {
				self->initiate_write();
			}
		});
	}

	std::shared_ptr<shared_state> state;
	websocket::stream<tcp::socket> socket;
	beast::flat_buffer buffer;
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
	http_session(tcp::socket socket, std::shared_ptr<shared_state> state) :
		socket(std::move(socket)),
		state(std::move(state))
	{}

	void initiate_read()
	{
		std::cout << "listening for http request..." << std::endl;
		auto request = std::make_shared<http::request<http::string_body>>();
		http::async_read(this->socket, buffer, *request, [self = shared_from_this(), request](error_code ec, std::size_t bytes) {
			self->on_read(request, bytes, ec);
		});
	}

private:
	template <class Response>
	void initiate_write(std::shared_ptr<Response> response)
	{
		response->set(http::field::server, BOOST_BEAST_VERSION_STRING);
		response->set(http::field::content_type, "text/html");
		response->prepare_payload();

		http::async_write(this->socket, *response, [self = shared_from_this(), response](error_code ec, std::size_t /* bytes */)
		{
			RETURN_ON_ERROR(ec);
			if (response->need_eof()) {
				self->socket.shutdown(tcp::socket::shutdown_send, ec);
			}
			else {
				self->initiate_read();
			}
		});
	}

	void on_read(std::shared_ptr<http::request<http::string_body>> request, std::size_t, error_code ec)
	{
		std::cout << "Processing http request..." << std::endl;

		// This means they closed the connection
		if (ec == http::error::end_of_stream) {
			std::cout << "Requested socket shutdown..." << std::endl;
			this->socket.shutdown(tcp::socket::shutdown_send, ec);
			return;
		}

		RETURN_ON_ERROR(ec);

		// See if it is a WebSocket Upgrade
		if (websocket::is_upgrade(*request)) {
			// Create a WebSocket session by transferring the socket
			std::cout << "Requested websocket upgrade..." << std::endl;
			return std::make_shared<websocket_session>(std::move(this->socket), this->state)->initiate_accept(request);
		}

		// Make sure we can handle the method
		if (request->method() != http::verb::get) {
			auto response = std::make_shared<http::response<http::string_body>>(http::status::bad_request, request->version());
			response->body() = "Unknown HTTP method";
			return this->initiate_write(std::move(response));
		}

		http::file_body::value_type body;
		body.open("./index.html", beast::file_mode::scan, ec);

		if (ec) {
			auto response = std::make_shared<http::response<http::string_body>>(http::status::internal_server_error, request->version());
			response->body() = ec.message();
			return this->initiate_write(std::move(response));
		}

		// Respond to GET request
		std::cout << "Requested index.html ..." << std::endl;
		auto response = std::make_shared<http::response<http::file_body>>(http::status::ok, request->version());
		response->body() = std::move(body);
		this->initiate_write(std::move(response));
	}

	tcp::socket socket;
	std::shared_ptr<shared_state> state;
	beast::flat_buffer buffer;
};


class listener : public std::enable_shared_from_this<listener> {
public:
	listener(net::io_context& ioc, std::shared_ptr<shared_state> state, uint16_t port) :
		socket(ioc),
		acceptor(ioc),
		state(std::move(state)),
		port(port)
	{
		tcp::endpoint endpoint{ net::ip::make_address("0.0.0.0"), port };

		THROW_ON_ERROR(acceptor.open(endpoint.protocol(), ec));
		THROW_ON_ERROR(acceptor.bind(endpoint, ec));
		THROW_ON_ERROR(acceptor.listen(net::socket_base::max_listen_connections, ec));
		acceptor.set_option(net::socket_base::reuse_address(true));
	}

	void run()
	{
		std::cout << "listening for incoming connection at port " << this->port << " " << std::endl;
		this->acceptor.async_accept(this->socket, [self = shared_from_this()](error_code ec) {
			RETURN_ON_ERROR(ec);
			std::make_shared<http_session>(std::move(self->socket), self->state)->initiate_read();
			std::cout << "http session created and initiated..." << std::endl;
			self->run();
		});
	}

	tcp::socket socket;
	tcp::acceptor acceptor;
	std::shared_ptr<shared_state> state;
	uint16_t port;
};


int main(int argc, char* argv[])
{
	bpo::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("port", bpo::value<uint16_t>(), "set port number");

	bpo::variables_map vm;
	bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
	bpo::notify(vm);

	if (!vm.count("port")) {
		throw std::runtime_error("port number was not set!");
	}

	auto port = vm["port"].as<uint16_t>();

	net::io_context ioc;
	auto state = std::make_shared<shared_state>();
	std::make_shared<listener>(ioc, state, port)->run();

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	net::signal_set signals(ioc, SIGINT, SIGTERM);
	signals.async_wait([&ioc](error_code const&, int) {
		ioc.stop();
		});

	ioc.run();

	return EXIT_SUCCESS;
}
