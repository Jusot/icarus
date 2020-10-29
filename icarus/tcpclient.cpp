#include <cassert>

#include "connector.hpp"
#include "tcpclient.hpp"
#include "eventloop.hpp"
#include "inetaddress.hpp"
#include "socketsfunc.hpp"

namespace icarus
{
TcpClient::TcpClient(EventLoop *loop, const InetAddress &server_addr, std::string name)
  : loop_(loop)
  , connector_(std::make_unique<Connector>(loop, server_addr))
  , name_(std::move(name))
  , connection_callback_(TcpConnection::default_connection_callback)
  , message_callback_(TcpConnection::default_message_callback)
  , retry_(false)
  , connect_(true)
  , next_conn_id_(1)
{
    connector_->set_new_connection_callback([this] (int sockfd) {
        this->new_connection(sockfd);
    });
}

TcpClient::~TcpClient()
{
    TcpConnectionPtr conn;
    bool unique = false;

    {
        /**
         * in fact, if we are inside different thread
         *  connection_ may be assigned after we assign conn
        */
        std::lock_guard lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }

    if (conn)
    {
        assert(loop_ == conn->get_loop());

        /**
         * reset close callback because old method cannot be used after destructor
        */
        loop_->run_in_loop([loop = loop_, conn] {
            conn->set_close_callback([loop] (const TcpConnectionPtr &conn) {
                loop->queue_in_loop([conn] {
                    conn->connect_destroyed();
                });
            });
        });

        if (unique)
        {
            /**
             * when conn is unique, the close_callback
             *  cannot be called if we do not force close it
            */
            conn->force_close();
        }
    }
    else
    {
        connector_->stop();
    }
}

void TcpClient::connect()
{
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_ = false;

    {
        std::lock_guard lock(mutex_);
        if (connection_)
        {
            connection_->shutdown();
        }
    }
}

/**
 * after some time, we can call stop directly
*/
void TcpClient::stop()
{
    connect_ = false;
    connector_->stop();
}

bool TcpClient::retry() const
{
    return retry_;
}

void TcpClient::enable_retry()
{
    retry_ = true;
}

const std::string &TcpClient::name() const
{
    return name_;
}

void TcpClient::set_connection_callback(ConnectionCallback cb)
{
    connection_callback_ = std::move(cb);
}

void TcpClient::set_message_callback(MessageCallback cb)
{
    message_callback_ = std::move(cb);
}

void TcpClient::set_write_complete_callback(WriteCompleteCallback cb)
{
    write_complete_callback_ = std::move(cb);
}

void TcpClient::new_connection(int sockfd)
{
    loop_->assert_in_loop_thread();

    InetAddress peer_addr(sockets::get_peer_addr(sockfd));
    InetAddress local_addr(sockets::get_local_addr(sockfd));

    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peer_addr.to_ip_port().c_str(), next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;

    auto conn = std::make_shared<TcpConnection>(
        loop_, conn_name, sockfd, local_addr, peer_addr
    );
    conn->set_connection_callback(connection_callback_);
    conn->set_message_callback(message_callback_);
    conn->set_write_complete_callback(write_complete_callback_);
    conn->set_close_callback([this] (const TcpConnectionPtr &conn) {
        this->remove_connection(conn);
    });

    {
        std::lock_guard lock(mutex_);
        connection_ = conn;
    }

    conn->connect_established();
}

void TcpClient::remove_connection(const TcpConnectionPtr &conn)
{
    loop_->assert_in_loop_thread();
    assert(loop_ == conn->get_loop());

    {
        std::lock_guard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queue_in_loop([conn] {
        conn->connect_destroyed();
    });

    if (retry_ && connect_)
    {
        connector_->restart();
    }
}
} // namespace icarus
