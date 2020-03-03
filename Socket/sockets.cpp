
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int server()
{
  boost::asio::io_service io_service;
  tcp::socket socket(io_service);

  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1234));
  acceptor.accept(socket);

  boost::asio::streambuf read_buffer;
  size_t bytes_read = boost::asio::read_until(socket, read_buffer, "\n");

  boost::asio::streambuf::const_buffers_type bufs = read_buffer.data();
  std::string message(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + bytes_read);
  std::cout << "server read this message: " << message << std::endl;

  boost::system::error_code error;
  boost::asio::write(socket, boost::asio::buffer("Hello From Server!\n"), error);
  if (error) {
    std::cerr << "server write failed: " << error.message() << std::endl;
    return 1;
  }
  return 0;
}


int client()
{
  boost::asio::io_service io_service;
  tcp::socket socket(io_service);

  socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 1234));

  // write message
  {
    boost::system::error_code error;
    boost::asio::write(socket, boost::asio::buffer("Hello from Client!\n"), error);
    if (error) {
      std::cerr << "client write failed: " << error.message() << std::endl;
      return 1;
    }
  }

  // read response
  {
    boost::system::error_code error;
    boost::asio::streambuf receive_buffer;
    size_t bytes_read = boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);

    if (error != boost::asio::error::eof) {
      std::cerr << "client read failed: " << error.message() << std::endl;
      return 1;
    }

    boost::asio::streambuf::const_buffers_type bufs = receive_buffer.data();
    std::string response(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + bytes_read);
    std::cout << "client read this response: " << response << std::endl;
    return 0;
  }
}


int main(int argc, char** argv)
{
  if (std::string(argv[1]) == "--server") {
    return server();
  }
  else if (std::string(argv[1]) == "--client") {
    return client();
  }
  return 0;
}
