#ifndef ICARUS_CALLBACKS_HPP
#define ICARUS_CALLBACKS_HPP

#include <memory>
#include <functional>

namespace icarus
{

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
} // namespace icarus

#endif // ICARUS_CALLBACKS_HPP