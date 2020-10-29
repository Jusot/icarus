#include "../icarus/poller.hpp"
#include "../icarus/tcpclient.hpp"
#include "../icarus/buffer.hpp"
#include "../icarus/eventloop.hpp"
#include "../icarus/tcpconnection.hpp"

#include <string>
#include <iostream>

using namespace std;
using namespace icarus;

class EchoClient
{
  public:
    EchoClient(EventLoop *loop, const InetAddress &server_addr)
      : client_(loop, server_addr, "echo client")
    {
        client_.set_connection_callback(on_connection);
        client_.set_message_callback(on_message);
    }

    void start()
    {
        client_.connect();
    }

  private:
    static void on_connection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            string str;
            cin >> str;
            conn->send(str);
        }
    }

    static void on_message(const TcpConnectionPtr &conn, Buffer *buf)
    {
        cout << buf->retrieve_all_as_string() << endl;
        string str;
        cin >> str;
        conn->send(str);
    }

    TcpClient client_;
};

int main()
{
    EventLoop loop;
    InetAddress server_addr(6666);
    EchoClient echo_client(&loop, server_addr);

    echo_client.start();
    loop.loop();

    return 0;
}
