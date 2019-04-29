#include <cassert>

#include "tcpconnection.hpp"
#include "socket.hpp"
#include "channel.hpp"
#include "eventloop.hpp"
#include "buffer.hpp"
#include "inetaddress.hpp"
#include "socketsfunc.hpp"

namespace icarus
{
TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& local_addr,
                             const InetAddress& peer_addr)
  : loop_(loop),
    name_(name),
    state_(kConnecting),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    local_addr_(local_addr),
    peer_addr_(peer_addr)
{
    channel_->set_read_callback([this] () {
        this->handle_read();
    });
    channel_->set_write_callback([this] () {
        this->handle_write();
    });
    channel_->set_close_callback([this] () {
        this->handle_close();
    });
    channel_->set_error_callback([this] () {
        this->handle_error();
    });
}

TcpConnection::~TcpConnection() = default;

EventLoop* TcpConnection::get_loop() const
{
    return loop_;
}

const std::string& TcpConnection::name() const
{
    return name_;
}

const InetAddress& TcpConnection::local_address() const
{
    return local_addr_;
}

const InetAddress& TcpConnection::peer_address() const
{
    return peer_addr_;
}

bool TcpConnection::connected() const
{
    return state_ == kConnected;
}

void TcpConnection::send(const void* message, int len)
{
    if (state_ == kConnected)
    {
        if (loop_->is_in_loop_thread())
        {
            send_in_loop(message, len);
        }
        else
        {
            std::string data(static_cast<const char*>(message), len);
            loop_->run_in_loop([this, d = std::move(data)] () {
                this->send_in_loop(d.data(), d.size());
            });
        }
    }
}

void TcpConnection::send(const std::string_view& message)
{
    if (state_ == kConnected)
    {
        if (loop_->is_in_loop_thread())
        {
            send_in_loop(message);
        }
    }
    else
    {
        loop_->run_in_loop([this, str = std::string(message.data(), message.size())] () {
            this->send_in_loop(static_cast<const void*>(str.data()), str.size());
        });
    }
}

void TcpConnection::send(Buffer* buf)
{
    if (state_ == kConnected)
    {
        if (loop_->is_in_loop_thread())
        {
            send_in_loop(buf->peek(), buf->readable_bytes());
            buf->retrieve_all();
        }
    }
    else
    {
        loop_->run_in_loop([this, str = buf->retrieve_all_as_string()] () {
            this->send_in_loop(str.data(), str.size());
        });
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        set_state(kDisconnecting);
        loop_->run_in_loop([this] () {
            this->shutdown_in_loop();
        });
    }
}

void TcpConnection::set_context(std::any context)
{
    context_ = std::move(context);
}

const std::any& TcpConnection::get_context() const
{
    return context_;
}

void TcpConnection::set_connection_callback(ConnectionCallback cb)
{
    connection_callback_ = std::move(cb);
}

void TcpConnection::set_message_callback(MessageCallback cb)
{
    message_callback_ = std::move(cb);
}

void TcpConnection::set_write_compelete_callback(WriteCompleteCallback cb)
{
    write_complete_callback_ = std::move(cb);
}

void TcpConnection::set_close_callback(CloseCallback cb)
{
    close_callback_ = std::move(cb);
}

void TcpConnection::connect_established()
{
    loop_->assert_in_loop_thread();
    assert(state_ == kConnecting);
    set_state(kConnected);
//    channel_->tie(shared_from_this());
    channel_->enable_reading();
    connection_callback_(shared_from_this());
}

void TcpConnection::connect_destroyed()
{
    loop_->assert_in_loop_thread();
    if (state_ == kConnected)
    {
        set_state(kDisconnected);
        channel_->disable_all();
        connection_callback_(shared_from_this());
    }
    loop_->remove_channel(channel_.get());
}

void TcpConnection::handle_read()
{
    loop_->assert_in_loop_thread();
    int saved_errno;
    ssize_t n = input_buffer_.read_fd(channel_->fd(), &saved_errno);
    if (n > 0)
    {
        message_callback_(shared_from_this(), &input_buffer_);
    }
    else if (n == 0)
    {
        handle_close();
    }
    else
    {
        // check saved_errno
    }
}

void TcpConnection::handle_write()
{
    loop_->assert_in_loop_thread();
    if (channel_->is_writing())
    {
        ssize_t n = sockets::write(channel_->fd(), output_buffer_.peek(), output_buffer_.readable_bytes());
        if (n > 0)
        {
            output_buffer_.retrieve(n);
            if (output_buffer_.readable_bytes() == 0)
            {
                channel_->disable_writing();
                if (write_complete_callback_)
                {
                    loop_->queue_in_loop([=, ptr = shared_from_this()] () {
                        write_complete_callback_(ptr);
                    });
                }
                if (state_ == kDisconnecting)
                {
                    shutdown_in_loop();
                }
            }
            else
            {
                // log
            }
        }
        else
        {
            abort();
        }
    }
    else
    {
        // log
    }
}

void TcpConnection::handle_close()
{
    loop_->assert_in_loop_thread();
    set_state(kDisconnected);
    channel_->disable_all();

    TcpConnectionPtr guard_this(shared_from_this());
    connection_callback_(guard_this);
    close_callback_(guard_this);
}

void TcpConnection::handle_error()
{
    // log error
}

void TcpConnection::send_in_loop(const std::string_view &message)
{
    send_in_loop(message.data(), message.size());
}

void TcpConnection::send_in_loop(const void *message, size_t len)
{
    loop_->assert_in_loop_thread();
    ssize_t nwrote = 0;

    if (!channel_->is_writing() && output_buffer_.readable_bytes() == 0)
    {
        nwrote = sockets::write(channel_->fd(), data, len);
        if (nwrote >= 0)
        {
            if (static_cast<size_t>(nwrote) < len)
            {
                // log
            }
            else if (write_complete_callback_)
            {
                loop_->queue_in_loop([=, ptr = shared_from_this()] () {
                    write_complete_callback_(shared_from_this());
                });
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                // log error
            }
        }
    }

    assert(nwrote >= 0);
    if (static_cast<size_t>(nwrote) < len)
    {
        output_buffer_.append(static_cast<const char *>(message) + nwrote, len - nwrote);
        if (!channel_->is_writing())
        {
            channel_->enable_reading();
        }
    }
}

void TcpConnection::shutdown_in_loop()
{
    loop_->assert_in_loop_thread();
    if (!channel_->is_writing())
    {
        socket_->shutdown_write();
    }
}

void TcpConnection::set_state(TcpConnection::States s)
{
    state_ = s;
}


} // namespace icarus
