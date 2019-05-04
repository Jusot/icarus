#include <utility>

#include <memory>
#include <cstdio>
#include <cassert>

#include "tcpserver.hpp"
#include "callbacks.hpp"
#include "eventloop.hpp"
#include "acceptor.hpp"
#include "inetaddress.hpp"
#include "socketsfunc.hpp"
#include "tcpconnection.hpp"
#include "eventloopthreadpool.hpp"

namespace
{
void default_connection_callback(const icarus::TcpConnectionPtr& conn)
{
    // TODO: log
}

void default_message_callback(const icarus::TcpConnectionPtr &, icarus::Buffer *buf)
{
    buf->retrieve_all();
}

} // namespace

namespace icarus
{

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr, std::string name)
  : loop_(loop),
    host_port_(listen_addr.to_ip_port()),
    name_(std::move(name)),
    acceptor_(new Acceptor(loop, listen_addr)),
    thread_pool_(new EventLoopThreadPool(loop)),
    connection_callback_(default_connection_callback),
    message_callback_(default_message_callback),
    started_(false),
    next_conn_id_(1)
{
    acceptor_->set_new_connection_callback([this] (int sockfd, const InetAddress& inet_addr) {
        this->new_connection(sockfd, inet_addr);
    });
}

TcpServer::~TcpServer()
{
    loop_->assert_in_loop_thread();
    for (auto& pair: connections_)
    {
        pair.second->get_loop()->run_in_loop([conn=pair.second] () {
            conn->connect_destroyed();
        });
    }
}

void TcpServer::set_thread_num(int num_threads)
{
    assert(num_threads >= 0);
    thread_pool_->set_thread_num(num_threads);
}

void TcpServer::start()
{
    if (!started_)
    {
        started_ = true;
        thread_pool_->start();
    }

    if (!acceptor_->listenning())
    {
        loop_->run_in_loop([ptr = acceptor_.get()] () {
            ptr->listen();
        });
    }
}

void TcpServer::set_connection_callback(ConnectionCallback cb)
{
    connection_callback_ = std::move(cb);
}

void TcpServer::set_message_callback(MessageCallback cb)
{
    message_callback_ = std::move(cb);
}

void TcpServer::new_connection(int sockfd, const InetAddress &peer_addr)
{
    loop_->assert_in_loop_thread();
    EventLoop* io_loop = thread_pool_->get_next_loop();
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", host_port_.c_str(), next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;
    InetAddress local_addr(sockets::get_local_addr(sockfd));
    auto conn = std::make_shared<TcpConnection>(io_loop, conn_name, sockfd, local_addr, peer_addr);
    connections_[conn_name] = conn;
    conn->set_connection_callback(connection_callback_);
    conn->set_message_callback(message_callback_);
    conn->set_write_complete_callback(write_complete_callback_);
    conn->set_close_callback([this] (const TcpConnectionPtr& p_conn) {
        this->remove_connection(p_conn);
    });
    io_loop->run_in_loop([conn] () {
        conn->connect_established();
    });
}

void TcpServer::remove_connection(const TcpConnectionPtr &conn)
{
    loop_->run_in_loop([this, conn] () {
        this->remove_connection_in_loop(conn);
    });
}

void TcpServer::remove_connection_in_loop(const TcpConnectionPtr &conn)
{
    loop_->assert_in_loop_thread();
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    loop_->queue_in_loop([=] () {
        conn->connect_destroyed();
    });
}

} // namespace icarus
