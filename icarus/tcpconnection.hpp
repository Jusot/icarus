#ifndef ICARUS_TCPCONNECTION_HPP
#define ICARUS_TCPCONNECTION_HPP

#include <string>
#include <string_view>
#include <memory>
#include <any>

#include "callbacks.hpp"
#include "noncopyable.hpp"
#include "inetaddress.hpp"
#include "buffer.hpp"

namespace icarus
{

class Channel;
class EventLoop;
class Socket;
class InetAddress;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
  public:
    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& local_addr,
                  const InetAddress& peer_addr);
    ~TcpConnection();

    EventLoop* get_loop() const;
    const std::string& name() const;
    const InetAddress& local_address() const;
    const InetAddress& peer_address() const;
    bool connected() const;

    void send(const void* message, int len);
    void send(const std::string_view& message);
    void send(Buffer* message);
    void shutdown();

    void set_context(std::any context);
    const std::any& get_context() const;

    void set_connection_callback(ConnectionCallback cb);
    void set_message_callback(MessageCallback cb);
    void set_write_compelete_callback(WriteCompleteCallback cb);
    void set_close_callback(CloseCallback cb);

    void connect_established();
    void connect_destroyed();

  private:
    enum States
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    void handle_read(/*Timestamp receiveTime*/);
    void handle_write();
    void handle_close();
    void handle_error();
    void send_in_loop(const std::string_view& message);
    void send_in_loop(const void* message, size_t len);
    void shutdown_in_loop();
    void set_state(States s);

    EventLoop* loop_;
    std::string name_;
    States state_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress local_addr_;
    InetAddress peer_addr_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    ConnectionCallback close_callback_;
    Buffer input_buffer_;
    Buffer output_buffer_;
    std::any context_;
};

} // namespace icarus

#endif // ICARUS_TCPCONNECTION_HPP