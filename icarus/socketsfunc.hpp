#ifndef ICARUS_SOCKHELPER_HPP
#define ICARUS_SOCKHELPER_HPP

#include <cstdint>
#include <sys/uio.h>

namespace icarus
{

namespace sockets
{

int create_nonblocking_or_die();
int connect(int sockfd, const struct sockaddr* addr);
void bind_or_die(int sockfd, const struct sockaddr* addr);
void listen_or_die(int sockfd);
int accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);
void shutdown_write(int sockfd);

void set_non_block_and_close_on_exec(int sockfd);

uint64_t host_to_network64(uint64_t hosst64);
uint32_t host_to_network32(uint32_t host32);
uint16_t host_to_network16(uint16_t host16);
uint64_t network_to_host64(uint64_t net64);
uint32_t network_to_host32(uint32_t net32);
uint16_t network_to_host16(uint16_t net16);

ssize_t write(int fd, const void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);

int get_socket_error(int sockfd);

struct sockaddr_in get_local_addr(int sockfd);
struct sockaddr_in get_peer_addr(int sockfd);
bool is_self_connect(int sockfd);

} // namespace sockets
} // namespace icarus

#endif // ICARUS_SOCKHELPER_HPP
