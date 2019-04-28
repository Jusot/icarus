#ifndef ICARUS_TCPCONNECTION_HPP
#define ICARUS_TCPCONNECTION_HPP

#include <string>
#include <string_view>
#include <memory>
#include <any>

#include "callbacks.hpp"
#include "noncopyable.hpp"

struct tcp_info;

namespace icarus
{

class Channel;
class Eventloop;
class Socket;
class InetAddress;

class TcpConnection : noncopyable
{
  public:
    TcpConnection(Eventloop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& local_addr,
                  const InetAddress& peer_addr);
    ~TcpConnection();

    Eventloop* get_loop() const;
    const std::string& name() const;
    const InetAddress& local_address() const;
    const InetAddress& peer_address() const;
    bool connected() const;
    bool disconnected() const;

    bool get_tcp_info(struct tcp_info*) const;
    std::string get_tcp_info_string() const;

    void send(const void* message, int len);
    void send(const std::string_view& message);
    void send(Buffer* message);
    void shutdown();
    void force_close();
    void force_close_with_delay(double seconds);
    void set_tcp_no_delay(bool on);

    void start_read();
    void stop_read();
    bool is_reading() const;

    void set_context(std::any context);
    const std::any& get_context() const;
    std::any* get_mutable_context();

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

    Eventloop* loop_;
    std::string name_;
    States state_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress local_addr_;
    InetAddress peer_addr;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback;
    WriteCompleteCallback write_complete_callback;
    ConnectionCallback CloseCallback_;
    Buffer input_buffer_;
    Buffer output_buffer_;
    std::any context_;
};

} // namespace icarus

#endif // ICARUS_TCPCONNECTION_HPP