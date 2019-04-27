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
    ~Channel();

    void handle_event();

    void set_read_callback(EventCallback cb);
    void set_write_callback(EventCallback cb);
    void set_close_callback(EventCallback cb);
    void set_error_callback(EventCallback cb);

    int fd() const;
    short events() const;
    void set_revents(short revents);
    bool is_none_event() const;

    void enable_reading();
    void disable_reading();
    void enable_writing();
    void disable_writing();
    void disable_all();
    bool is_writing() const;
    bool is_reading() const;

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
    int        index_;
    const int  fd_;
    short      events_;
    short      revents_;

    bool event_handling_;
    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;
};
}

#endif // ICARUS_CHANNEL_HPP
