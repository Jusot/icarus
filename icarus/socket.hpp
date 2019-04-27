#ifndef ICARUS_SOCKET_HPP
#define ICARUS_SOCKET_HPP

#include "noncopyable.hpp"

namespace icarus
{

class InetAddress;

class Socket : noncopyable
{
  public:
    explicit Socket(int sockfd);
    ~Socket();

    int fd() const;
    bool get_tcp_info(struct tcp_info*) const;
    bool get_tcp_info_string(char* buf, int len) const;

    void bind_address(const InetAddress& localaddr);
    void listen();

    int accept(InetAddress* peeraddr);

    void shutdown_write();

    void set_tcp_no_delay(bool on);
    void set_reuse_addr(bool on);
    void set_reuse_port(bool on);
    void set_keep_alive(bool on);

  private:
    const int sockfd_;
};

} // namespace icarus

#endif // ICARUS_SOCKET_HPP
