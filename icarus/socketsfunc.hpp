#ifndef ICARUS_SOCKHELPER_HPP
#define ICARUS_SOCKHELPER_HPP

namespace icarus
{

namespace sockets
{

void bindOrDie(int sockfd, const struct sockaddr* addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in* addr);
void close(int sockfd);
void shutdownWrite(int sockfd);

} // namespace sockets
} // namespace icarus

#endif // ICARUS_SOCKHELPER_HPP