#include <cassert>

#include <poll.h>

#include "channel.hpp"
#include "eventloop.hpp"

using namespace icarus;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop),
    index_(-1),
    fd_(fd),
    events_(0),
    revents_(0),
    event_handling_(false)
{
    // ...
}

Channel::~Channel()
{
    assert(!event_handling);
}

void Channel::handle_event()
{
    event_handling_ = true;

    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        if (close_callback_)
        {
            close_callback_();
        }
    })
    if (revents_ & POLLNVAL)
    {
        // ...
    }
    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (error_callback_)
        {
            error_callback_();
        }
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (read_callback_)
        {
            read_callback_();
        }
    }
    if (revents_ & POLLOUT)
    {
        if (write_callback_)
        {
            write_callback_();
        }
    }
    
    event_handling_ = false;
}

void Channel::set_read_callback(EventCallback cb)
{
    read_callback_ = std::move(cb);
}

void Channel::set_write_callback(EventCallback cb)
{
    write_callback_ = std::move(cb);
}

void Channel::set_close_callback(EventCallback cb)
{
    close_callback_ = std::move(cb);
}

void Channel::set_error_callback(EventCallback cb)
{
    error_callback_ = std::move(cb);
}

int Channel::fd() const
{
    return fd_;
}

short Channel::events() const
{
    return events_;
}

void Channel::set_revents(short revents)
{
    revents_ = revents;
}

bool Channel::is_none_event() const
{
    return events_ == kNoneEvent;
}

void Channel::enable_reading()
{
    events_ |= kReadEvent;
    update();
}

void Channel::disable_reading()
{
    events_ &= ~kReadEvent;
    update();
}

void Channel::enable_writing()
{
    events_ |= kWriteEvent;
    update();
}

void Channel::disable_writing()
{
    events_ &= ~kWriteEvent;
    update();
}

void Channel::disable_all()
{
    events_ = kNoneEvent;
    update();
}

bool Channel::is_writing() const
{
    return events_ & kWriteEvent;
}

bool Channel::is_reading() const
{
    return events_ & kReadEvent;
}

int Channel::index()
{
    return index_;
}

void Channel::set_index(int index)
{
    index_ = index;
}

EventLoop *Channel::owner_loop()
{
    return loop_;
}

void Channel::update()
{
    loop_->update_channel(this);
}