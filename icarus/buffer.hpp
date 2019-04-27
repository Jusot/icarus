#ifndef ICARUS_BUFFER_HPP
#define ICARUS_BUFFER_HPP

#include <vector>
#include <string>
#include <string_view>
#include <cstdint>

namespace icarus
{
class Buffer
{
  public:
    static constexpr size_t kCheapPrepend = 8;
    static constexpr size_t kInitialSize = 1024;

    explicit Buffer(size_t initial_size = kInitialSize);

    void swap(Buffer& rhs);

    size_t readable_bytes() const;
    size_t writable_bytes() const;
    size_t prependable_bytes() const;

    const char* peek() const;

    const char* findCRLF() const;
    const char* findCRLF(const char* start) const;
    const char* findEOL() const;
    const char* findEOL(const char* start) const;

    void retrieve(size_t len);
    void retrieve_until(const char* end);
    void retrieve_int64();
    void retrieve_int32();
    void retrieve_int16();
    void retrieve_int8();
    void retrieve_all();

    std::string retrieve_all_as_string();
    std::string retrieve_as_string(size_t len);
    std::string_view to_string_view() const;

    void append(const std::string_view& str);
    void append(const char* data, size_t len);
    void append(const void* data, size_t len);

    void ensure_writable_bytes(size_t len);

    char* begin_write();
    const char* begin_write() const;

    void has_written(size_t len);
    void unwrite(size_t len);

    void append_int64(int64_t x);
    void append_int32(int32_t x);
    void append_int16(int16_t x);
    void append_int8(int8_t x);

    int64_t read_int64();
    int32_t read_int32();
    int16_t read_int16();
    int8_t  read_int8();

    int64_t peek_int64() const;
    int32_t peek_int32() const;
    int16_t peek_int16() const;
    int8_t  peek_int8() const;

    void prepend_int64(int64_t x);
    void prepend_int32(int32_t x);
    void prepend_int16(int16_t x);
    void prepend_int8(int8_t x);
    void prepend(const void* data, size_t len);

    void shrink(size_t reserve);

    size_t internal_capacity() const;

    ssize_t read_fd(int fd, int* saved_errno);

  private:
    char* begin();
    const char* begin() const;
    void make_space(size_t len);

  private:
    std::vector<char> buffer_;
    size_t reader_index_;
    size_t writer_index_;

    static const char kCRLF[];
};
} // namespace icarus

#endif // ICARUS_BUFFER_HPP
