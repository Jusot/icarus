#include <cassert>

#include "connector.hpp"
#include "channel.hpp"
#include "socketsfunc.hpp"
#include "eventloop.hpp"

namespace icarus
{
Connector::Connector(EventLoop *loop, const InetAddress &server_addr)
  : loop_(loop), server_addr_(server_addr)
  , connect_(false), state_(kDisconnected)
{
    // ...
}

Connector::~Connector()
{
    assert(!channel_);
}

void Connector::set_new_connection_callback(NewConnectionCallback cb)
{
    new_connection_callback_ = std::move(cb);
}

void Connector::start()
{
    connect_ = true;
    loop_->run_in_loop([this]{ this->start_in_loop(); });
}

void Connector::restart()
{
    loop_->assert_in_loop_thread();
    set_state(kDisconnected);
    connect_ = true;
    start_in_loop();
}

void Connector::stop()
{
    connect_ = false;
    loop_->queue_in_loop([this]{ this->stop_in_loop(); });
}

const InetAddress &Connector::server_addr() const
{
    return server_addr_;
}

void Connector::set_state(States state)
{
    state_ = state;
}

void Connector::start_in_loop()
{
    loop_->assert_in_loop_thread();
    assert(connect_);
    assert(state_ == kDisconnected);
    connect();
}

void Connector::stop_in_loop()
{
    loop_->assert_in_loop_thread();
    if (state_ == kConnecting)
    {
        set_state(kDisconnected);
        int sockfd = detach();
        close(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = sockets::create_nonblocking_or_die();
    int ret = sockets::connect(sockfd, server_addr_.get_sock_addr());
    int saved_errno = (ret == 0) ? 0 : errno;

    switch (saved_errno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        close(sockfd);
        break;

    default:
        close(sockfd);
        break;
    }
}

void Connector::connecting(int sockfd)
{
    set_state(kConnecting);
    assert(!channel_);

    channel_.reset(new Channel(loop_, sockfd));
    channel_->set_write_callback([this] { this->handle_write(); });
    channel_->set_error_callback([this] { this->handle_error(); });
    channel_->enable_writing();
}

void Connector::handle_write()
{
    assert(state_ == kConnecting);

    int sockfd = detach();
    int err = sockets::get_socket_error(sockfd);
    if (err)
    {
        // log
        retry(sockfd);
    }
    else if (sockets::is_self_connect(sockfd))
    {
        // log
        retry(sockfd);
    }
    else
    {
        set_state(kConnected);
        if (connect_)
        {
            new_connection_callback_(sockfd);
        }
        else
        {
            close(sockfd);
        }
    }
}

void Connector::handle_error()
{
    if (state_ == kConnecting)
    {
        // log
        int sockfd = detach();
        retry(sockfd);
    }
}

void Connector::close(int sockfd)
{
    sockets::close(sockfd);
    set_state(kDisconnected);
}

void Connector::retry(int sockfd)
{
    close(sockfd);
    if (connect_)
    {
        loop_->queue_in_loop([this] { this->start_in_loop(); });
    }
}

int Connector::detach()
{
    channel_->disable_all();
    channel_->remove();
    int sockfd = channel_->fd();
    /**
     * this is unsafe because it may be executed before `return sockfd`
     *  and we are inside Channel::handle_event
    */
    loop_->queue_in_loop([this]{ this->reset_channel(); });
    return sockfd;
}

void Connector::reset_channel()
{
    channel_.reset();
}
} // namespace icarus
