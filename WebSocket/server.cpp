#include <Scheme/Scheme.h>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>

#include <queue>
#include <memory>
#include <string>
#include <iostream>
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;
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

	std::unordered_set<websocket_session*> sessions;
	uint16_t port;
	std::string root;
};


class websocket_session : public std::enable_shared_from_this<websocket_session> {
public:
	websocket_session(tcp::socket socket, std::shared_ptr<shared_state> state) :
		stream(std::move(socket)),
		state(std::move(state))
	{
		scm::fun_ptr test = [this](const scm::List& lst) {
			std::cout << "function test called on server" << std::endl;
			this->write(R"((print "server told me to print this"))");
			return nullptr;
		};

		scm::env_ptr app_env = std::make_shared<scm::Env>();
		app_env->inner.insert({ "test", test });

		this->env = scm::global_env();
		this->env->outer = app_env;
	}

	~websocket_session()
	{
		this->state->leave(this);
		std::cout << "session closed..." << std::endl;
	}

	void accept(std::shared_ptr<http::request<http::string_body>> request)
	{
		std::cout << "Accepting websocket upgrade..." << std::endl;
		auto self = shared_from_this();

		this->stream.async_accept(*request, [self](error_code ec) {
			RETURN_ON_ERROR(ec);
			self->state->join(self.get());
			self->read();
			});
	}

	void read()
	{
		auto self = shared_from_this();
		auto buffer = std::make_shared<beast::flat_buffer>();

		this->stream.async_read(*buffer, [self, buffer](error_code ec, std::size_t bytes) {
			RETURN_ON_ERROR(ec);
			auto message = beast::buffers_to_string(buffer->data());
			std::cout << "read message: " << message << std::endl;

			std::any exp = scm::read(message.begin(), message.end());
			exp = scm::eval(exp, self->env);
			//std::stringstream ss;
			//scm::print(exp, ss);
			//std::cout << "writing message: " << ss.str() << std::endl;
			//self->write(ss.str());
			self->read();
			});
	}

	void write(const std::string& message)
	{
		this->write(std::make_shared<std::string>(message));
	}

	void write(std::shared_ptr<std::string> message)
	{
		this->queue.push(message);
		if (this->queue.size() == 1) {
			this->write();
		}
	}

	void write()
	{
		auto self = shared_from_this();
		this->stream.async_write(net::buffer(*this->queue.front()), [self](error_code ec, std::size_t bytes) {
			RETURN_ON_ERROR(ec);
			self->queue.pop();

			if (!self->queue.empty()) {
				self->write();
			}
			});
	}

	void broadcast(const std::string& message)
	{
		auto ss = std::make_shared<std::string>(message);
		for (auto session : this->state->sessions) {
			session->write(ss);
		}
	}

	std::shared_ptr<shared_state> state;
	websocket::stream<tcp::socket> stream;
	std::queue<std::shared_ptr<std::string>> queue;
	std::map<std::string, bool> connections;
	std::string username;
	scm::env_ptr env;
};


class http_session : public std::enable_shared_from_this<http_session> {
public:
	http_session(tcp::socket socket, std::shared_ptr<shared_state> state) :
		socket(std::move(socket)),
		state(std::move(state))
	{}

	void read()
	{
		std::cout << "listening for http request..." << std::endl;

		auto self = shared_from_this();
		auto buffer = std::make_shared<beast::flat_buffer>();
		auto request = std::make_shared<http::request<http::string_body>>();

		http::async_read(this->socket, *buffer, *request, [self, buffer, request](error_code ec, std::size_t bytes) {
			self->on_read(request, bytes, ec);
			});
	}

private:
	template <class Response>
	void write(std::shared_ptr<Response> response)
	{
		response->set(http::field::server, BOOST_BEAST_VERSION_STRING);
		response->prepare_payload();
		auto self = shared_from_this();

		http::async_write(this->socket, *response, [self, response](error_code ec, std::size_t) {
			RETURN_ON_ERROR(ec);
			if (response->need_eof()) {
				self->socket.shutdown(tcp::socket::shutdown_send, ec);
				std::cout << "connection closed..." << std::endl;
			}
			else {
				self->read();
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
			std::make_shared<websocket_session>(std::move(this->socket), this->state)->accept(request);
		}
	}

	tcp::socket socket;
	std::shared_ptr<shared_state> state;
};


class listener : public std::enable_shared_from_this<listener> {
public:
	listener(net::io_context& ioc, std::shared_ptr<shared_state> state) :
		socket(ioc),
		acceptor(ioc),
		state(std::move(state))
	{
		tcp::endpoint endpoint{ net::ip::make_address("0.0.0.0"), this->state->port };

		THROW_ON_ERROR(acceptor.open(endpoint.protocol(), ec));
		THROW_ON_ERROR(acceptor.bind(endpoint, ec));
		THROW_ON_ERROR(acceptor.listen(net::socket_base::max_listen_connections, ec));
		acceptor.set_option(net::socket_base::reuse_address(true));
	}

	void run()
	{
		std::cout << "listening for incoming connection at port " << this->state->port << " " << std::endl;
		auto self = shared_from_this();

		this->acceptor.async_accept(this->socket, [self](error_code ec) {
			RETURN_ON_ERROR(ec);
			std::make_shared<http_session>(std::move(self->socket), self->state)->read();
			std::cout << "http session created and initiated..." << std::endl;
			self->run();
			});
	}

	tcp::socket socket;
	tcp::acceptor acceptor;
	std::shared_ptr<shared_state> state;
};

int main(int argc, char* argv[])
{
	try {
		bpo::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("port", bpo::value<uint16_t>(), "set port number");

		bpo::variables_map vm;
		bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
		bpo::notify(vm);

		if (!vm.count("port")) {
			std::cerr << "port number was not set!" << std::endl;
			return EXIT_FAILURE;
		}

		auto state = std::make_shared<shared_state>();
		state->port = vm["port"].as<uint16_t>();

		net::io_context ioc;
		std::make_shared<listener>(ioc, state)->run();

		// Capture SIGINT and SIGTERM to perform a clean shutdown
		net::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](error_code const&, int) {
			ioc.stop();
			});

		ioc.run();
	}
	catch (std::exception e) {
		std::cerr << e.what();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
