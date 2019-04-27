#ifndef ICARUS_CALLBACKS_HPP
#define ICARUS_CALLBACKS_HPP

#include <memory>
#include <functional>

namespace icarus
{

// class Buffer;
class TcpConnection;
class Buffer;

using TcpConnectionPtr      = std::shared_ptr<TcpConnection>;
using TimerCallback         = std::function<void()>;
using ConnectionCallback    = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback         = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback       = std::function<void (const TcpConnectionPtr&,
                                                  Buffer* /*,
                                                  Timestamp*/)>;
void default_connection_callback(const TcpConnectionPtr& conn);
void default_message_callback(const TcpConnectionPtr& conn,
                              Buffer* buffer /*, 
                              Timestamp receive_time */);

} // namespace icarus

#endif // ICARUS_CALLBACKS_HPP