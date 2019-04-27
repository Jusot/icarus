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

InetAddress::InetAddress(uint16_t port, bool loopback_only)
{
    in_addr_t net_ip = ::htonl(loopback_only ? INADDR_LOOPBACK : INADDR_ANY);
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

std::string InetAddress::to_ip_port() const
{
    return to_ip().append(":").append(std::to_string(to_port()));
}

std::string InetAddress::to_ip() const
{
    char buf[64] = "";

    if (::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf)) == 0)
        abort();

    return std::string(buf);
}

uint16_t InetAddress::to_port() const
{
    return ::ntohs(addr_.sin_port);
}

const struct sockaddr *InetAddress::get_sock_addr() const
{
    return reinterpret_cast<const struct sockaddr *>(&addr_);
}

void InetAddress::set_sock_addr(const struct sockaddr_in& addr)
{
    addr_ = addr;
}

uint32_t InetAddress::ip_net_endian() const
{
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::port_net_endian() const
{
    return addr_.sin_port;
}

} // namespace icarus
