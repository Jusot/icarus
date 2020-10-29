#include "../icarus/tcpserver.hpp"
#include "../icarus/buffer.hpp"
#include "../icarus/eventloop.hpp"
#include "../icarus/tcpconnection.hpp"

#include <string>
#include <iostream>

using namespace std;
using namespace icarus;

class EchoServer
{
  public:
    EchoServer(EventLoop* loop, const InetAddress& listen_addr)
      : server_(loop, listen_addr, "echo server")
    {
        server_.set_message_callback(on_message);
    }

    void start()
    {
        server_.start();
    }

  private:
    static void on_message(const TcpConnectionPtr& conn, Buffer* buf)
    {
        conn->send(buf);
    }

    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress listen_addr(6666);
    EchoServer echo_server(&loop, listen_addr);

    echo_server.start();
    loop.loop();

    return 0;
}
