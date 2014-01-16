#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace std;
namespace asio = boost::asio;
namespace ip = asio::ip;
const char CHAT_MESSAGE_ENDING[2] = "\n";

struct invalid_arguments {};

int main(int argc, const char* const argv[]) {
  char port[5] = "";
  strcpy(port, argv[1]);

  asio::io_service io_service;
  ip::tcp::socket sock(io_service);

  try {
    cout << "establish server at: " << port << "\n";
    ip::tcp::acceptor acceptor( io_service, ip::tcp::endpoint( ip::tcp::v4(), atoi(port) ) );
    acceptor.accept(sock);

    string buffer;
    while (true) {
      asio::streambuf receive_buffer;
      boost::system::error_code error;
      asio::read(sock, receive_buffer, asio::transfer_at_least(1), error);
      if (error && error != asio::error::eof)
      {
          std::cout << "receive failed: " << error.message() << std::endl;
      }
      else if (asio::buffer_cast<const char*>
                (receive_buffer.data()) == string(CHAT_MESSAGE_ENDING)) //メッセージの最終文字は"\n"
      {
          break;
      }
    }
    return 0;
  } catch (...) {
    cerr << "some error occurred;" << endl;
  }
}
