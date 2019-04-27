#ifndef ICARUS_SOCKHELPER_HPP
#define ICARUS_SOCKHELPER_HPP

#include <cstdint>
#include <sys/uio.h>

namespace icarus
{

namespace sockets
{

void bindOrDie(int sockfd, const struct sockaddr* addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);
void shutdownWrite(int sockfd);

uint64_t hostToNetwork64(uint64_t hosst64);
uint32_t hostToNetwork32(uint32_t host32);
uint16_t hostToNetwork16(uint16_t host16);
uint64_t networkToHost64(uint64_t net64);
uint32_t networkToHost32(uint32_t net32);
uint16_t networkToHost16(uint16_t net16);

ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);

} // namespace sockets
} // namespace icarus

#endif // ICARUS_SOCKHELPER_HPP