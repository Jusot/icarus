#ifndef ICARUS_EVENTLOOP_HPP
#define ICARUS_EVENTLOOP_HPP

namespace icarus
{
class EventLoop : noncopyable
{
  public:
    EventLoop();

    ~EventLoop();

    void loop();

    void assert_in_loop_thread();

    bool is_in_loop_thread() const;

    static EventLoop *get_event_loop_of_current_thread();

  private:
    void abort_not_in_loop_thread();

    bool looping_;
    const std::thread::id thread_id_;
};
} // namespace icarus

#endif // ICARUS_EVENTLOOP_HPP