#ifndef ICARUS_INETADDRESS_HPP
#define ICARUS_INETADDRESS_HPP

#include <netinet/in.h>
#include <string>

namespace icarus
{

// Wrapper of struct sockaddr_in. Only support IPv4 now.
class InetAddress
{
  public:
    explicit InetAddress(uint16_t port = 0, bool loopback_only = false);
    InetAddress(const char *ip, uint16_t port);
    explicit InetAddress(const struct sockaddr_in &addr);

    sa_family_t family() const;
    std::string to_ip_port() const;
    std::string to_ip() const;
    uint16_t to_port() const;

    const struct sockaddr *get_sock_addr() const;
    void set_sock_addr(const struct sockaddr_in& addr);

    uint32_t ip_net_endian() const;
    uint16_t port_net_endian() const;

    // static bool resolve(const char *hostname, InetAddress *result);

  private:
    struct sockaddr_in addr_;
};

} // namespace icaurs

#endif // ICARUS_INETADDRESS_HPP
