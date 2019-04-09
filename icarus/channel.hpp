#ifndef ICARUS_CHANNEL_HPP
#define ICARUS_CHANNEL_HPP

#include <functional>

#include "noncopyable.hpp"

namespace icarus
{
class EventLoop;

class Channel : noncopyable
{
  public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop *loop, int fd);

    void handleEvent();

    void set_read_callback(EventCallback cb);
    void set_write_callback(EventCallback cb);
    void set_error_callback(EventCallback cb);

    int fd() const;
    int events() const;
    void set_revents(int revents);
    bool is_none_event() const;

    void enable_reading();
    void enable_writing();
    void disable_writing();
    void disable_all();

    // for Poller
    int index();
    void set_index(int index);

    EventLoop *owner_loop();

  private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;
    const int  fd_;
    int        events_;
    int        revents_;
    int        index_;

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;
};
}

#endif // ICARUS_CHANNEL_HPP
