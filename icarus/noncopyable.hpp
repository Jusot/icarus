#ifndef ICARUS_NONCOPYABLE_HPP
#define ICARUS_NONCOPYABLE_HPP

namespace icarus
{
// base class, disallow copying
class noncopyable
{
  protected:
    constexpr noncopyable() = default;
    ~noncopyable() = default;

  private:
    noncopyable(const noncopyable &) = delete;
    noncopyable & operator=(const noncopyable &) = delete;
};

} // namespace icarus

#endif // ICARUS_NONCOPYABLE_HPP
