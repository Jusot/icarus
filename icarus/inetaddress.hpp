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
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);
    InetAddress(const char *ip, uint16_t port);
    explicit InetAddress(const struct sockaddr_in &addr);

    sa_family_t family() const;
    std::string toIpPort() const;
    std::string toIp() const;
    uint16_t toPort() const;

    const struct sockaddr *getSockaddr() const;
    void setSockAddr(const struct sockaddr_in& addr);

    uint32_t ipNetEndian() const;
    uint16_t portNetEndian() const;

    // static bool resolve(const char *hostname, InetAddress *result);

  private:
    struct sockaddr_in addr_;
};

} // namespace icaurs

#endif // ICARUS_INETADDRESS_HPP
