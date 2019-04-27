#include <algorithm>
#include <cassert>
#include <cstring>

#include "socketsfunc.hpp"
#include "buffer.hpp"

namespace icarus
{
const char Buffer::kCRLF[] = "\r\n";

Buffer::Buffer(size_t initialize_size)
  : buffer_(kCheapPrepend + initialize_size),
    reader_index_(kCheapPrepend),
    writer_index_(kCheapPrepend)
{
    assert(readableBytes() == 0);
    assert(writableBytes() == 0);
    assert(prependableBytes() == 0);
}

void Buffer::swap(Buffer& rhs)
{
    buffer_.swap(rhs.buffer_);
    std::swap(reader_index_, rhs.reader_index_);
    std::swap(writer_index_, rhs.writer_index_);
}

size_t Buffer::readableBytes() const
{
    return writer_index_ - reader_index_;
}

size_t Buffer::writableBytes() const
{
    return buffer_.size() - writer_index_;
}

size_t Buffer::prependableBytes() const
{
    return reader_index_;
}

const char* Buffer::peek() const
{
    return begin() + reader_index_;
}

const char* Buffer::findCRLF() const
{
    const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
}

const char* Buffer::findCRLF(const char* start) const
{
    assert(peek() <= start);
    assert(start <= beginWrite());
    const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
}

const char* Buffer::findEOL() const
{
    const void* eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char*>(eol);
}

const char* Buffer::findEOL(const char* start) const
{
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void* eol = memchr(start, '\n', beginWrite() - start);
    return static_cast<const char*>(eol);
}

void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    if (len < readableBytes())
    {
        reader_index_ += len;
    }
    else
    {
        retrieveAll();
    }
}

void Buffer::retrieveUntil(const char* end)
{
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
}

void Buffer::retrieveInt64()
{
    retrieve(sizeof(int64_t));
}

void Buffer::retrieveInt32()
{
    retrieve(sizeof(int32_t));
}

void Buffer::retrieveInt16()
{
    retrieve(sizeof(int16_t));
}

void Buffer::retrieveInt8()
{
    retrieve(sizeof(int8_t));
}

void Buffer::retrieveAll()
{
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend;
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(size_t len)
{
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

std::string_view Buffer::toStringView() const
{
    return std::string_view(peek(), readableBytes());
}

void Buffer::append(const std::string_view& str)
{
    append(str.data(), str.size());
}

void Buffer::append(const char* data, size_t len)
{
    ensureWritableBytes(len);
    std::copy(data, data+len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const void* data, size_t len)
{
    append(static_cast<const char*>(data), len);
}

void Buffer::ensureWritableBytes(size_t len)
{
    if (writableBytes() < len)
    {
        makeSpace(len);
    }
    assert(writableBytes() >= len);
}

char* Buffer::beginWrite()
{
    return begin() + writer_index_;
}

const char* Buffer::beginWrite() const
{
    return begin() + writer_index_;
}

void Buffer::hasWritten(size_t len)
{
    assert(len <= writableBytes());
    writer_index_ += len;
}

void Buffer::unwrite(size_t len)
{
    assert(len <= readableBytes());
    writer_index_ -= len;
}

void Buffer::appendInt64(int64_t x)
{
    int64_t be64 = sockets::hostToNetwork64(x);
    append(static_cast<const void*>(&be64), sizeof(be64));
}

void Buffer::appendInt32(int32_t x)
{
    int32_t be32 = sockets::hostToNetwork32(x);
    append(static_cast<const void*>(&be32), sizeof(be32));
}

void Buffer::appendInt16(int16_t x)
{
    int16_t be16 = sockets::hostToNetwork16(x);
    append(static_cast<const void*>(&be16), sizeof(be16));
}

void Buffer::appendInt8(int8_t x)
{
    append(static_cast<const void*>(&x), sizeof(x));
}

int64_t Buffer::readInt64()
{
    int64_t result = peekInt64();
    retrieveInt64();
    return result;
}

int32_t Buffer::readInt32()
{
    int32_t result = peekInt32();
    retrieveInt32();
    return result;
}

int16_t Buffer::readInt16()
{
    int16_t result = peekInt16();
    retrieveInt16();
    return result;
}

int8_t Buffer::readInt8()
{
    int8_t result = peekInt8();
    retrieveInt8();
    return result;
}

int64_t Buffer::peekInt64() const
{
    assert(readableBytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    ::memcpy(&be64, peek(), sizeof(be64));
    return sockets::networkToHost64(be64);
}

int32_t Buffer::peekInt32() const
{
    assert(readableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof(be32));
    return sockets::networkToHost32(be32);
}

int16_t Buffer::peekInt16() const
{
    assert(readableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof(be16));
    return sockets::networkToHost16(be16);
}

int8_t Buffer::peekInt8() const
{
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
}


void Buffer::prependInt64(int64_t x)
{
    int64_t be64 = sockets::hostToNetwork64(x);
    prepend(&be64, sizeof(be64));
}

void Buffer::prependInt32(int32_t x)
{
    int32_t be32 = sockets::hostToNetwork32(x);
    prepend(&be32, sizeof(be32));
}

void Buffer::prependInt16(int16_t x)
{
    int16_t be16 = sockets::hostToNetwork16(x);
    prepend(&be16, sizeof(be16));
}

void Buffer::prependInt8(int8_t x)
{
    prepend(&x, sizeof(x));
}

void Buffer::prepend(const void* data, size_t len)
{
    assert(len <= prependableBytes());
    reader_index_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d+len, begin()+reader_index_);
}

void Buffer::shrink(size_t reserve)
{
    Buffer other;
    other.ensureWritableBytes(readableBytes()+reserve);
    other.append(toStringView());
    swap(other);
}

size_t Buffer::internalCapacity() const
{
    return buffer_.capacity();
}

ssize_t Buffer::readFd(int fd, int* saved_errno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();

    vec[0].iov_base = begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = sockets::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *saved_errno = errno;
    }
    else if (static_cast<size_t>(n) <= writable)
    {
        writer_index_ += n;
    }
    else
    {
        writer_index_ = buffer_.size();
        append(extrabuf, n-writable);
    }

    return n;
}

char* Buffer::begin()
{
    return buffer_.data();
}

const char* Buffer::begin() const
{
    return buffer_.data();
}

void Buffer::makeSpace(size_t len)
{
    if (writableBytes() + prependableBytes() < len + kCheapPrepend)
    {
        buffer_.resize(writer_index_+len);
    }
    else
    {
        assert(kCheapPrepend < reader_index_);
        size_t readable = readableBytes();
        std::copy(begin()+reader_index_,
                  begin()+writer_index_,
                  begin()+kCheapPrepend);
        reader_index_ = kCheapPrepend;
        writer_index_ = reader_index_ + readable;
        assert(readable == readableBytes());
    }
}

} // namespace icarus