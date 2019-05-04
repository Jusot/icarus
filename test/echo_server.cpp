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
    EchoServer(EventLoop* loop, const InetAddress& listen_addr);
    void start();
  private:
    static void on_message(const TcpConnectionPtr& conn, Buffer* buf);
    TcpServer server_;
};

EchoServer::EchoServer(EventLoop* loop, const InetAddress& listen_addr)
  : server_(loop, listen_addr, "echo server")
{
    server_.set_message_callback([] (const TcpConnectionPtr& conn, Buffer* buf) {
        EchoServer::on_message(conn, buf);
    });
}

void EchoServer::start()
{
    server_.start();
}

void EchoServer::on_message(const TcpConnectionPtr &conn, Buffer *buf)
{
    conn->send(buf);
}

int main()
{
//    std::cout << "he" << std::endl;
    EventLoop loop;
    InetAddress addr(6666);
    EchoServer echo_server(&loop, addr);

    echo_server.start();
    loop.loop();
}
