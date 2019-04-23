#include <cstring>
#include <cassert>
#include <string>
#include <arpa/inet.h>

#include "inetaddress.hpp"

#define INIT_ADDR(ip, port)                 \
    do                                      \
    {                                       \
        ::memset(&addr_, 0, sizeof(addr_)); \
        addr_.sin_family = AF_INET;         \
        addr_.sin_addr.s_addr = (ip);       \
        addr_.sin_port = (port);            \
    } while (0)

namespace icarus
{

InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
    in_addr_t net_ip = ::htonl(loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY);
    in_port_t net_port = ::htons(port);
    INIT_ADDR(net_ip, net_port);
}

InetAddress::InetAddress(const char *ip, uint16_t port)
{
    in_addr_t net_ip = inet_addr(ip);
    assert(net_ip != INADDR_NONE);
    in_port_t net_port = ::htons(port);
    INIT_ADDR(net_ip, net_port);
}

#undef INIT_ADDR

InetAddress::InetAddress(const struct sockaddr_in &addr)
    : addr_(addr)
{
}

sa_family_t InetAddress::family() const
{
    return addr_.sin_family;
}

std::string InetAddress::toIpPort() const
{
    return toIp().append(":").append(std::to_string(toPort()));
}

std::string InetAddress::toIp() const
{
    char buf[64] = "";

    if (::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf)) == 0)
        abort();

    return std::string(buf);
}

uint16_t InetAddress::toPort() const
{
    return ::ntohs(addr_.sin_port);
}

const struct sockaddr *InetAddress::getSockaddr() const
{
    return reinterpret_cast<const struct sockaddr *>(&addr_);
}

uint32_t InetAddress::ipNetEndian() const
{
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::portNetEndian() const
{
    return addr_.sin_port;
}

} // namespace icarus
