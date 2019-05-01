#include <unistd.h>
#include <cstdio>

#include "../icarus/acceptor.hpp"
#include "../icarus/inetaddress.hpp"
#include "../icarus/socketsfunc.hpp"
#include "../icarus/eventloop.hpp"

using namespace icarus;
using namespace std;

void new_connection(int sockfd, const InetAddress &peer_addr);

int main()
{
    printf("main(): pid = %ld\n", (long)getpid());
    InetAddress listen_addr(9981);
    EventLoop loop;

    Acceptor acceptor(&loop, listen_addr);
    acceptor.set_new_connection_callback(new_connection);
    acceptor.listen();

    loop.loop();
}


void new_connection(int sockfd, const InetAddress &peer_addr)
{
    printf("new_connection(): accepted a new conenction from %s\n",
           peer_addr.to_ip_port().c_str());
    ::write(sockfd, "How are you?\n", 13);
    sockets::close(sockfd);
}
