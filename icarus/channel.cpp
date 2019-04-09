#include <poll.h>

#include "eventloop.hpp"
#include "channel.hpp"

namespace icarus
{
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1)
{
    // ...
}

void Channel::handleEvent()
{
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
}

void Channel::set_read_callback(const EventCallback& cb)
{
    read_callback_ = cb;
}

void Channel::set_write_callback(const EventCallback& cb)
{
    write_callback_ = cb;
}

void Channel::set_error_callback(const EventCallback& cb)
{
    error_callback_ = cb;
}

int Channel::fd() const
{
    return fd_;
}

int Channel::events() const
{
    return events_;
}

void Channel::set_revents(int revents)
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
    // loop_->update_channel(this);
}
}