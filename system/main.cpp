#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "network/connect.hpp"
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

using namespace std;
namespace asio = boost::asio;
namespace ip = asio::ip;
const char CHAT_MESSAGE_ENDING[2] = "\n";

struct invalid_arguments {};

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

class Connection : DependsOnOUEngine {
public:
  Connection( const std::string &hostname, uint16_t port, const boost::shared_ptr< config > &configs ) {
    auto network_ = new oneup::NetworkService( 10 );
    oneup::TaskPtr task( network_ );
    oneup::Engine::getInstance()->add( TASK_ID_NETWORK_SERVICE, task, 16 );
    client = network_->connect( hostname.c_str(), port );
    client->setEventListener( new event_receiver( configs, client->getId() ) );
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

boost::shared_ptr< Connection > mrs_connection;

void connect_mrs_server() {
  mrs_connection = 
    boost::shared_ptr< Connection > conn( new Connection(
      get_context().configs->get_server_addr().authority->host,
      *get_context().configs->get_server_addr().authority->port,
      get_context().configs
    ) );
}

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
      if (error && error != asio::error::eof) {
          std::cout << "receive failed: " << error.message() << std::endl;
      }
      else if (asio::buffer_cast<const char*>
                (receive_buffer.data()) == string(CHAT_MESSAGE_ENDING)) { //メッセージの最終文字は"\n"
          break;
      }
    }
    return 0;
  } catch (...) {
    cerr << "some error occurred;" << endl;
  }
}
