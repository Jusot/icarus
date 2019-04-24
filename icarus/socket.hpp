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

    int fd() const;
    bool getTcpInfo(struct tcp_info*) const;
    bool getTcpinfoString(char* buf, int len) const;

    void bindAddress(const InetAddress& localaddr);
    void listen();

    int accept(InetAddress* peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

  private:
    const int sockfd_;
};

} // namespace icarus

#endif // ICARUS_SOCKET_HPP
