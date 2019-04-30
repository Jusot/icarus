#ifndef ICARUS_ACCEPTOR_HPP
#define ICARUS_ACCEPTOR_HPP

#include <functional>

#include "noncopyable.hpp"
#include "channel.hpp"
#include "socket.hpp"

namespace icarus
{

class Acceptor
{
  public:
    using NewConnectionCallback = std::function<void (int sockfd,
                                                      const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listen_addr);

    void set_new_connection_callback(NewConnectionCallback cb);

    bool listenning() const;
    void listen();

  private:
    void handle_read();

    EventLoop* loop_;
    Socket accept_socket_;
    Channel accept_channel_;
    NewConnectionCallback new_connection_callback_;
    bool listenning_;
};

} // namespace icarus

#endif //ICARUS_ACCEPTOR_HPP
