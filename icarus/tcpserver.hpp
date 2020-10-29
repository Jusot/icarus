#ifndef ICARUS_TCPSERVER_HPP
#define ICARUS_TCPSERVER_HPP

#include <unordered_map>
#include <string>
#include <memory>

#include "noncopyable.hpp"
#include "inetaddress.hpp"
#include "callbacks.hpp"

namespace icarus
{

class InetAddress;
class EventLoop;
class Acceptor;
class EventLoopThreadPool;

class TcpServer : noncopyable
{
  public:
    TcpServer(EventLoop* loop, const InetAddress& listen_addr, std::string name);
    ~TcpServer();

    void set_thread_num(int num_threads);
    void start();

    void set_connection_callback(ConnectionCallback cb);
    void set_message_callback(MessageCallback cb);
    void set_write_complete_callback(WriteCompleteCallback cb);

  private:
    void new_connection(int sockfd, const InetAddress& peer_addr);
    void remove_connection(const TcpConnectionPtr& conn);
    void remove_connection_in_loop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;
    const std::string host_port_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> thread_pool_;
    ConnectionCallback connection_callback_;
    MessageCallback  message_callback_;
    WriteCompleteCallback write_complete_callback_;
    bool started_;
    int next_conn_id_;
    ConnectionMap connections_;
};

} // namespace icarus

#endif //ICARUS_TCPSERVER_HPP
