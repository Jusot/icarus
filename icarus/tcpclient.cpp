#include "connector.hpp"
#include "tcpclient.hpp"
#include "eventloop.hpp"
#include "inetaddress.hpp"

namespace icarus
{
TcpClient::TcpClient(EventLoop *loop, const InetAddress &server_addr, std::string name)
  : loop_(loop)
  , connector_(new Connector(loop, server_addr))
  , name_(std::move(name))
  , connection_callback_(TcpConnection::default_connection_callback)
  , message_callback_(TcpConnection::default_message_callback)
  , retry_(false)
  , connect_(true)
  , next_conn_id_(1)
{
    // ...
}
} // namespace icarus
