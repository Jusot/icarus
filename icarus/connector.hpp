#ifndef ICARUS_TCPCLIENT_HPP
#define ICARUS_TCPCLIENT_HPP

#include <memory>
#include <functional>

#include "noncopyable.hpp"
#include "inetaddress.hpp"

namespace icarus
{
class Channel;
class EventLoop;

class Connector : noncopyable
{
  public:
    using NewConnectionCallback
        = std::function<void (int sockfd)>;

    Connector(EventLoop *loop, const InetAddress &server_addr);
    ~Connector();

    void set_new_connection_callback(NewConnectionCallback cb);

    void start();
    void restart();
    void stop();

    const InetAddress &server_addr() const;

  private:
    enum States { kDisconnected, kConnecting, kConnected };

    void set_state(States state);
    void start_in_loop();
    void stop_in_loop();
    void connect();
    void connecting(int sockfd);
    void handle_write();
    void handle_error();
    void close(int sockfd);
    void retry(int sockfd);
    int detach();
    void reset_channel();

    EventLoop *loop_;
    InetAddress server_addr_;
    bool connect_;
    States state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback new_connection_callback_;
};
} // namespace icarus

#endif
