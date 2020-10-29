#ifndef ICARUS_TCPCLIENT_HPP
#define ICARUS_TCPCLIENT_HPP

#include <string>
#include <memory>

#include "noncopyable.hpp"
#include "tcpconnection.hpp"

namespace icarus
{

class InetAddress;
class EventLoop;
class Connector;

class TcpClient : noncopyable
{
  public:
    TcpClient(EventLoop *loop, const InetAddress &server_addr, std::string name);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    bool retry() const;
    void enable_retry();

    const std::string &name() const;

    void set_connection_callback(ConnectionCallback cb);
    void set_message_callback(MessageCallback cb);
    void set_write_complete_callback(WriteCompleteCallback cb);

  private:
    void new_connection(int sockfd);
    void remove_connection(const TcpConnectionPtr &conn);

    EventLoop *loop_;
    std::unique_ptr<Connector> connector_;
    const std::string name_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    bool retry_;
    bool connect_;
    int next_conn_id_;
    TcpConnectionPtr connection_;
};
} // namespace icarus

#endif
