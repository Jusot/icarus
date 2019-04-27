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
    assert(readable_bytes() == 0);
    assert(writable_bytes() == 0);
    assert(prependable_bytes() == 0);
}

void Buffer::swap(Buffer& rhs)
{
    buffer_.swap(rhs.buffer_);
    std::swap(reader_index_, rhs.reader_index_);
    std::swap(writer_index_, rhs.writer_index_);
}

size_t Buffer::readable_bytes() const
{
    return writer_index_ - reader_index_;
}

size_t Buffer::writable_bytes() const
{
    return buffer_.size() - writer_index_;
}

size_t Buffer::prependable_bytes() const
{
    return reader_index_;
}

const char* Buffer::peek() const
{
    return begin() + reader_index_;
}

const char* Buffer::findCRLF() const
{
    const char* crlf = std::search(peek(), begin_write(), kCRLF, kCRLF + 2);
    return crlf == begin_write() ? nullptr : crlf;
}

const char* Buffer::findCRLF(const char* start) const
{
    assert(peek() <= start);
    assert(start <= begin_write());
    const char* crlf = std::search(start, begin_write(), kCRLF, kCRLF + 2);
    return crlf == begin_write() ? nullptr : crlf;
}

const char* Buffer::findEOL() const
{
    const void* eol = memchr(peek(), '\n', readable_bytes());
    return static_cast<const char*>(eol);
}

const char* Buffer::findEOL(const char* start) const
{
    assert(peek() <= start);
    assert(start <= begin_write());
    const void* eol = memchr(start, '\n', begin_write() - start);
    return static_cast<const char*>(eol);
}

void Buffer::retrieve(size_t len)
{
    assert(len <= readable_bytes());
    if (len < readable_bytes())
    {
        reader_index_ += len;
    }
    else
    {
        retrieve_all();
    }
}

void Buffer::retrieve_until(const char* end)
{
    assert(peek() <= end);
    assert(end <= begin_write());
    retrieve(end - peek());
}

void Buffer::retrieve_int64()
{
    retrieve(sizeof(int64_t));
}

void Buffer::retrieve_int32()
{
    retrieve(sizeof(int32_t));
}

void Buffer::retrieve_int16()
{
    retrieve(sizeof(int16_t));
}

void Buffer::retrieve_int8()
{
    retrieve(sizeof(int8_t));
}

void Buffer::retrieve_all()
{
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend;
}

std::string Buffer::retrieve_all_as_string()
{
    return retrieve_as_string(readable_bytes());
}

std::string Buffer::retrieve_as_string(size_t len)
{
    assert(len <= readable_bytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

std::string_view Buffer::to_string_view() const
{
    return std::string_view(peek(), readable_bytes());
}

void Buffer::append(const std::string_view& str)
{
    append(str.data(), str.size());
}

void Buffer::append(const char* data, size_t len)
{
    ensure_writable_bytes(len);
    std::copy(data, data+len, begin_write());
    has_written(len);
}

void Buffer::append(const void* data, size_t len)
{
    append(static_cast<const char*>(data), len);
}

void Buffer::ensure_writable_bytes(size_t len)
{
    if (writable_bytes() < len)
    {
        make_space(len);
    }
    assert(writable_bytes() >= len);
}

char* Buffer::begin_write()
{
    return begin() + writer_index_;
}

const char* Buffer::begin_write() const
{
    return begin() + writer_index_;
}

void Buffer::has_written(size_t len)
{
    assert(len <= writable_bytes());
    writer_index_ += len;
}

void Buffer::unwrite(size_t len)
{
    assert(len <= readable_bytes());
    writer_index_ -= len;
}

void Buffer::append_int64(int64_t x)
{
    int64_t be64 = sockets::host_to_network64(x);
    append(static_cast<const void*>(&be64), sizeof(be64));
}

void Buffer::append_int32(int32_t x)
{
    int32_t be32 = sockets::host_to_network32(x);
    append(static_cast<const void*>(&be32), sizeof(be32));
}

void Buffer::append_int16(int16_t x)
{
    int16_t be16 = sockets::host_to_network16(x);
    append(static_cast<const void*>(&be16), sizeof(be16));
}

void Buffer::append_int8(int8_t x)
{
    append(static_cast<const void*>(&x), sizeof(x));
}

int64_t Buffer::read_int64()
{
    int64_t result = peek_int64();
    retrieve_int64();
    return result;
}

int32_t Buffer::read_int32()
{
    int32_t result = peek_int32();
    retrieve_int32();
    return result;
}

int16_t Buffer::read_int16()
{
    int16_t result = peek_int16();
    retrieve_int16();
    return result;
}

int8_t Buffer::read_int8()
{
    int8_t result = peek_int8();
    retrieve_int8();
    return result;
}

int64_t Buffer::peek_int64() const
{
    assert(readable_bytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    ::memcpy(&be64, peek(), sizeof(be64));
    return sockets::network_to_host64(be64);
}

int32_t Buffer::peek_int32() const
{
    assert(readable_bytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof(be32));
    return sockets::network_to_host32(be32);
}

int16_t Buffer::peek_int16() const
{
    assert(readable_bytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof(be16));
    return sockets::network_to_host16(be16);
}

int8_t Buffer::peek_int8() const
{
    assert(readable_bytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
}


void Buffer::prepend_int64(int64_t x)
{
    int64_t be64 = sockets::host_to_network64(x);
    prepend(&be64, sizeof(be64));
}

void Buffer::prepend_int32(int32_t x)
{
    int32_t be32 = sockets::host_to_network32(x);
    prepend(&be32, sizeof(be32));
}

void Buffer::prepend_int16(int16_t x)
{
    int16_t be16 = sockets::host_to_network16(x);
    prepend(&be16, sizeof(be16));
}

void Buffer::prepend_int8(int8_t x)
{
    prepend(&x, sizeof(x));
}

void Buffer::prepend(const void* data, size_t len)
{
    assert(len <= prependable_bytes());
    reader_index_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d+len, begin()+reader_index_);
}

void Buffer::shrink(size_t reserve)
{
    Buffer other;
    other.ensure_writable_bytes(readable_bytes()+reserve);
    other.append(to_string_view());
    swap(other);
}

size_t Buffer::internal_capacity() const
{
    return buffer_.capacity();
}

ssize_t Buffer::read_fd(int fd, int* saved_errno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writable_bytes();

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

void Buffer::make_space(size_t len)
{
    if (writable_bytes() + prependable_bytes() < len + kCheapPrepend)
    {
        buffer_.resize(writer_index_+len);
    }
    else
    {
        assert(kCheapPrepend < reader_index_);
        size_t readable = readable_bytes();
        std::copy(begin()+reader_index_,
                  begin()+writer_index_,
                  begin()+kCheapPrepend);
        reader_index_ = kCheapPrepend;
        writer_index_ = reader_index_ + readable;
        assert(readable == readable_bytes());
    }
}

} // namespace icarus