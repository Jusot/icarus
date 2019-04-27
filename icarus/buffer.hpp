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

    size_t readableBytes() const;
    size_t writableBytes() const;
    size_t prependableBytes() const;

    const char* peek() const;

    const char* findCRLF() const;
    const char* findCRLF(const char* start) const;
    const char* findEOL() const;
    const char* findEOL(const char* start) const;

    void retrieve(size_t len);
    void retrieveUntil(const char* end);
    void retrieveInt64();
    void retrieveInt32();
    void retrieveInt16();
    void retrieveInt8();
    void retrieveAll();

    std::string retrieveAllAsString();
    std::string retrieveAsString(size_t len);
    std::string_view toStringView() const;

    void append(const std::string_view& str);
    void append(const char* data, size_t len);
    void append(const void* data, size_t len);

    void ensureWritableBytes(size_t len);

    char* beginWrite();
    const char* beginWrite() const;

    void hasWritten(size_t len);
    void unwrite(size_t len);

    void appendInt64(int64_t x);
    void appendInt32(int32_t x);
    void appendInt16(int16_t x);
    void appendInt8(int8_t x);

    int64_t readInt64();
    int32_t readInt32();
    int16_t readInt16();
    int8_t  readInt8();

    int64_t peekInt64() const;
    int32_t peekInt32() const;
    int16_t peekInt16() const;
    int8_t  peekInt8() const;

    void prependInt64(int64_t x);
    void prependInt32(int32_t x);
    void prependInt16(int16_t x);
    void prependInt8(int8_t x);
    void prepend(const void* data, size_t len);

    void shrink(size_t reserve);

    size_t internalCapacity() const;

    ssize_t readFd(int fd, int* saved_errno);

  private:
    char* begin();
    const char* begin() const;
    void makeSpace(size_t len);

  private:
    std::vector<char> buffer_;
    size_t reader_index_;
    size_t writer_index_;

    static const char kCRLF[];
};
} // namespace icarus

#endif // ICARUS_BUFFER_HPP
