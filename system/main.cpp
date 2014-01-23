#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "system/config.hpp"
#include "system/config.hpp"
#include "rpc/MRSClient.h"

#include "network/NetworkService.h"
#include "network/Client.h"
#include "network/ConnectionState.h"
#include "network/Serializer.h"
#include "network/Transporter.h"
#include "network/NetworkEventListener.h"

const oneup::TaskId TASK_ID_NETWORK_SERVICE = 1000;

using boost::asio::ip::tcp;

class session { //ウェブソケットサーバとのやり取り実装
public:
  session(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error) {
      boost::asio::async_write(socket_,
          boost::asio::buffer(data_, bytes_transferred),
          boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error));
    }
    else {
      delete this;
    }
  }

  void handle_write(const boost::system::error_code& error) {
    if (!error) {
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
          boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else {
      delete this;
    }
  }

private:
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};


class server { //ウェブソケットサーバと接続TCPサーバ
public:
  server(boost::asio::io_service& io_service, short port)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
  {
    session* new_session = new session(io_service_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          boost::asio::placeholders::error));
  }

  void handle_accept(session* new_session,
      const boost::system::error_code& error)
  {
    if (!error) {
      new_session->start();
      new_session = new session(io_service_);
      std::cout << "session handle accept" << std::endl;
      acceptor_.async_accept(new_session->socket(),
          boost::bind(&server::handle_accept, this, new_session,
            boost::asio::placeholders::error));
    }
    else {
      delete new_session;
    }
  }

private:
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
};


class DependsOnOUEngine {
private:
  class OUE_ {
  public:
    static const OUE_ &init() {
       static const auto instance = OUE_();
       return instance;
    }
  private:
    OUE_() {
      oneup::Engine::createInstance();
      oneup::LoggingServicePtr pLoggingService = oneup::Engine::getInstance()->search<oneup::LoggingService>(oneup::Engine::TASK_ID_LOGGING_SERVICE);
      pLoggingService->setOutputPriority(oneup::LoggingService::LOG_PRIORITY_DEBUG);
    }
  };
public:
  DependsOnOUEngine() {
    OUE_::init();
  }
};

struct context {
	CMRSGmContract* contract;
};

context &get_context();
context& get_context() {
	static context instance = context();
	return instance;
}

class Connection : DependsOnOUEngine {
public:
  Connection( const std::string &hostname, uint16_t port, const boost::shared_ptr< config > &configs ) {
    auto network_ = new oneup::NetworkService( 10 );
    oneup::TaskPtr task( network_ );
    oneup::Engine::getInstance()->add( TASK_ID_NETWORK_SERVICE, task, 16 );
    client = network_->connect( hostname.c_str(), port );
    client->addContract( "MRSGMPROTO", new CMRSGmContract() );
    client->getTransporter().setSocket( new oneup::foundation::TCPSocket() );
    oneup::TaskId client_id = client->getId();
    auto network = oneup::Engine::getInstance()->search<oneup::NetworkService>(TASK_ID_NETWORK_SERVICE);
    auto connection = network->search<oneup::Connection>( client_id );
    auto contract = connection->getContract<CMRSGmContract>( "MRSGMPROTO" );
    get_context().contract = contract;
  }
  void run() {
    oneup::Engine::getInstance()->update( 0 );
  }
private:
  oneup::ClientPtr client;
};

int main(int argc, const char* const argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: chat_tool <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;
    server s(io_service, std::atoi(argv[1]));
    io_service.run();
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
