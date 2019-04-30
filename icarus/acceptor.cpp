#include "acceptor.hpp"
#include "socketsfunc.hpp"
#include "eventloop.hpp"
#include "inetaddress.hpp"

namespace icarus
{

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listen_addr)
  : loop_(loop),
    accept_socket_(sockets::create_nonblocking_or_die()),
    accept_channel_(loop, accept_socket_.fd()),
    listenning_(false)
{
    accept_socket_.set_reuse_addr(true);
    accept_socket_.bind_address(listen_addr);
    accept_channel_.set_read_callback([this] () {
       this->handle_read();
    });
}

void Acceptor::set_new_connection_callback(NewConnectionCallback cb)
{
    new_connection_callback_ = std::move(cb);
}

bool Acceptor::listenning() const
{
    return listenning_;
}

void Acceptor::listen()
{
    loop_->assert_in_loop_thread();
    listenning_ = true;
    accept_socket_.listen();
    accept_channel_.enable_reading();
}

void Acceptor::handle_read()
{
    loop_->assert_in_loop_thread();
    InetAddress peer_addr(0);
    int connfd = accept_socket_.accept(&peer_addr);
    if (connfd >= 0)
    {
        if (new_connection_callback_)
        {
            new_connection_callback_(connfd, peer_addr);
        }
        else
        {
            sockets::close(connfd);
        }
    }
}

} // namespace icarus
