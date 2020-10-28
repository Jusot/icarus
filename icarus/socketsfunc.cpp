#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <endian.h>

#include "socketsfunc.hpp"

namespace icarus::sockets
{
int create_nonblocking_or_die()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        abort();
    }

    return sockfd;
}

int connect(int sockfd, const struct sockaddr *addr)
{
    return ::connect(sockfd, addr, static_assert<socklen_t>(sizeof(struct sockaddr_in)));
}

void bind_or_die(int sockfd, const struct sockaddr *addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr)));
    if (ret < 0)
    {
        abort();
    }
}

void listen_or_die(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0)
    {
        abort();
    }
}

int accept(int sockfd, struct sockaddr_in *addr)
{
    auto addrlen = static_cast<socklen_t>(sizeof(*addr));
    int connfd = ::accept4(sockfd, reinterpret_cast<struct sockaddr*>(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0)
    {
        int savedErrno = errno;
        switch (savedErrno)
        {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO: // ???
        case EPERM:
        case EMFILE: // per-process lmit of open file desctiptor ???
            // expected errors
            errno = savedErrno;
            break;
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            abort();
            break;
        default:
            abort();
            break;
        }
    }
    return connfd;
}

void shutdown_write(int sockfd)
{
    if (::shutdown(sockfd, SHUT_WR) < 0)
    {
        // TODO: log error
    }
}

void close(int sockfd)
{
    if (::close(sockfd) < 0)
    {
        abort();
    }
}

void set_non_block_and_close_on_exec(int sockfd)
{
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);

    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
}

uint64_t host_to_network64(uint64_t host64)
{
    return htobe64(host64);
}

uint32_t host_to_network32(uint32_t host32)
{
    return htobe32(host32);
}

uint16_t host_to_network16(uint16_t host16)
{
    return htobe16(host16);
}

uint64_t network_to_host64(uint64_t net64)
{
    return be64toh(net64);
}

uint32_t network_to_host32(uint32_t net32)
{
    return be32toh(net32);
}

uint16_t network_to_host16(uint16_t net16)
{
    return be16toh(net16);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    return ::write(fd, buf, count);
}

ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}

int get_socket_error(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

struct sockaddr_in get_local_addr(int sockfd)
{
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof localaddr);
    socklen_t addrlen = sizeof localaddr;
    if (::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&localaddr), &addrlen) < 0)
    {
        // TODO: log error
    }
    return localaddr;
}

struct sockaddr_in get_peer_addr(int sockfd)
{
    struct sockaddr_in peeraddr;
    memset(&peeraddr, 0, sizeof peeraddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, reinterpret_cast<struct sockaddr*>(&peeraddr), &addrlen) < 0)
    {
        // TODO: log error
    }
    return peeraddr;
}

bool is_self_connect(int sockfd)
{
    struct sockaddr_in localaddr = getLocalAddr(sockfd);
    struct sockaddr_in peeraddr = getPeerAddr(sockfd);

    const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
    const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);

    return laddr4->sin_port == raddr4->sin_port
        && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
}

} // namespace icarus
