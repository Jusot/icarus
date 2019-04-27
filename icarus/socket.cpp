#include <cstring>
#include <cstdio>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include "socket.hpp"
#include "inetaddress.hpp"
#include "socketsfunc.hpp"

namespace icarus
{
Socket::Socket(int sockfd)
    : sockfd_(sockfd)
{
}

int Socket::fd() const
{
    return sockfd_;
}

bool Socket::get_tcp_info(struct tcp_info *tcp_info) const
{
    socklen_t len = sizeof(*tcp_info);
    ::memset(tcp_info, 0, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcp_info, &len) == 0;
}

bool Socket::get_tcp_info_string(char *buf, int len) const
{
    struct tcp_info tcp_info;
    bool ok = get_tcp_info(&tcp_info);
    if (ok)
    {
        snprintf(buf, len, "unrecovered=%u "
                           "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                           "lost=%u retrans=%u rtt=%u rttvar=%u "
                           "sshthresh=%u cwnd=%u total_retrans=%u",
                 tcp_info.tcpi_retransmits, // Number of unrecovered [RTO] timeouts
                 tcp_info.tcpi_rto,         // Retransmit timeout in usec
                 tcp_info.tcpi_ato,         // Predicted tick of soft clock in usec
                 tcp_info.tcpi_snd_mss,
                 tcp_info.tcpi_rcv_mss,
                 tcp_info.tcpi_lost,    // Lost packets
                 tcp_info.tcpi_retrans, // Retransmitted packets out
                 tcp_info.tcpi_rtt,     // Smoothed round trip time in usec
                 tcp_info.tcpi_rttvar,  // Medium deviation
                 tcp_info.tcpi_snd_ssthresh,
                 tcp_info.tcpi_snd_cwnd,
                 tcp_info.tcpi_total_retrans); // Total retransmits for entire connection
    }
    return ok;
}

void Socket::bind_address(const InetAddress &localaddr)
{
    sockets::bind_or_die(sockfd_, localaddr.get_sock_addr());
}

void Socket::listen()
{
    sockets::listen_or_die(sockfd_);
}

int Socket::accept(InetAddress *peeraddr)
{
    struct sockaddr_in addr;
    ::memset(&addr, 0, sizeof(addr));
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0)
    {
        peeraddr->set_sock_addr(addr);
    }
    return connfd;
}

void Socket::shutdown_write()
{
    sockets::shutdown_write(sockfd_);
}

#define SET_TCP_OPTION(LEVEL, OPT_NAME)                       \
    do                                                        \
    {                                                         \
        int optval = on ? 1 : 0;                              \
        ::setsockopt(sockfd_, LEVEL, OPT_NAME, &optval,       \
                     static_cast<socklen_t>(sizeof(optval))); \
    } while (0)

void Socket::set_tcp_no_delay(bool on)
{
    SET_TCP_OPTION(IPPROTO_TCP, TCP_NODELAY);
}

void Socket::set_reuse_addr(bool on)
{
    SET_TCP_OPTION(SOL_SOCKET, SO_REUSEADDR);
}

void Socket::set_reuse_port(bool on)
{
    SET_TCP_OPTION(SOL_SOCKET, SO_REUSEPORT);
}

void Socket::set_keep_alive(bool on)
{
    SET_TCP_OPTION(SOL_SOCKET, SO_KEEPALIVE);
}
#undef SET_TCP_OPTION

} // namespace icarus
