
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

#include <json.hpp>
using json = nlohmann::json;

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
	{}

	~websocket_session()
	{
		this->state->leave(this);
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
			auto message_string = beast::buffers_to_string(buffer->data());
			std::cout << "read message: " << message_string << std::endl;

			json json_message = json::parse(message_string);

			if (json_message.is_null()) {
				std::cout << "invalid message, disconnecting..." << std::endl;
				return;
			}

			if (!json_message.contains("type")) {
				std::cout << "message must contain type..." << std::endl;
				return;
			}

			if (!json_message.contains("username")) {
				std::cout << "message must contain username..." << std::endl;
				return;
			}

			if (json_message["type"] == "username") {
				self->username = json_message["username"];

				std::vector<std::string> userlist;
				for (auto session : self->state->sessions) {
					if (session->username != self->username) {
						userlist.push_back(session->username);
					}
					else {
						json new_user;
						new_user["type"] = "new-user";
						new_user["username"] = self->username;
					}
				}

				json json_userlist;
				json_userlist["type"] = "userlist";
				json_userlist["users"] = userlist;

				self->write(json_userlist.dump());
			}
			else {
				std::string receiver_name = json_message["username"];
				json_message["username"] = self->username;

				auto receiver_session = [receiver_name, self]() {
					for (auto session : self->state->sessions) {
						if (session->username == receiver_name) {
							return session;
						}
					}
					throw std::runtime_error("receiver session not found!");
				}();

				if (!self->connections.contains(receiver_name)) {
					if (receiver_session->connections.contains(self->username)) {
						throw std::runtime_error("receiver session should not contain out username!");
					}
					self->connections[receiver_name] = false;
					receiver_session->connections[self->username] = true;
				}

				json_message["polite"] = receiver_session->connections.at(self->username);
				receiver_session->write(json_message.dump());
			}

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

		http::async_write(this->socket, *response, [self, response](error_code ec, std::size_t)
			{
				RETURN_ON_ERROR(ec);
				if (response->need_eof()) {
					self->socket.shutdown(tcp::socket::shutdown_send, ec);
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
			return;
		}

		// Make sure we can handle the method
		if (request->method() != http::verb::get) {
			auto response = std::make_shared<http::response<http::string_body>>(http::status::bad_request, request->version());
			response->body() = "Unknown HTTP method";
			response->set(http::field::content_type, "text/html");
			this->write(std::move(response));
			return;
		}

		fs::path path(this->state->root + std::string(request->target()));

		if (!path.has_filename()) {
			path.append("index.html");
		}

		http::file_body::value_type body;
		body.open(path.string().c_str(), beast::file_mode::scan, ec);

		if (ec) {
			auto response = std::make_shared<http::response<http::string_body>>(http::status::internal_server_error, request->version());
			response->body() = ec.message();
			response->set(http::field::content_type, "text/html");
			this->write(std::move(response));
			return;
		}

		// Respond to GET request
		auto mime_type = [](const std::filesystem::path& path) {
			if (path.extension() == ".css") return "text/css";
			return "text/html";
		};

		auto response = std::make_shared<http::response<http::file_body>>(http::status::ok, request->version());
		response->body() = std::move(body);
		response->set(http::field::content_type, mime_type(path));
		this->write(std::move(response));
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
			("port", bpo::value<uint16_t>(), "set port number")
			("root", bpo::value<std::string>(), "public folder");

		bpo::variables_map vm;
		bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
		bpo::notify(vm);

		if (!vm.count("port")) {
			std::cerr << "port number was not set!" << std::endl;
			return EXIT_FAILURE;
		}
		if (!vm.count("root")) {
			std::cerr << "root folder was not set!" << std::endl;
			return EXIT_FAILURE;
		}

		auto state = std::make_shared<shared_state>();
		state->port = vm["port"].as<uint16_t>();
		state->root = vm["root"].as<std::string>();

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
