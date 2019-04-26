#include <cstdlib>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <endian.h>

#include "socketsfunc.hpp"

namespace icarus
{
namespace sockets
{
void bindOrDie(int sockfd, const struct sockaddr *addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr)));
    if (ret < 0)
    {
        abort();
    }
}

void listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0)
    {
        abort();
    }
}

int accept(int sockfd, struct sockaddr_in *addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
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

void shutdownWrite(int sockfd)
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

uint64_t hostToNetwork64(uint64_t host64)
{
    return htobe64(host64);
}

uint32_t hostToNetwork32(uint32_t host32)
{
    return htobe32(host32);
}

uint16_t hostToNetwork16(uint16_t host16)
{
    return htobe16(host16);
}

uint64_t networkToHost64(uint64_t net64)
{
    return be64toh(net64);
}

uint32_t networkToHost32(uint32_t net32)
{
    return be32toh(net32);
}

uint16_t networkToHost16(uint16_t net16)
{
    return be16toh(net16);
}

} // namespace sockets
} // namespace icarus
