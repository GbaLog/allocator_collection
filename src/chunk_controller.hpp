#ifndef CHUNK_CONTROLLER_HPP
#define CHUNK_CONTROLLER_HPP

#include "static_chunk_allocator.hpp"
#include "ac_concepts.hpp"
#include <cstddef>
#include <algorithm>
#include <cstring>
#include <iostream>

namespace ac
{

// The work of this class is to allocate and manipulate the chunks of memory.
// It provides easy interface to write and read data to/from it.

template<IsChunkAllocator Allocator>
class chunk_controller
{
public:
  using allocator_type = Allocator;
  using chunk_type = typename Allocator::chunk_type;

  explicit
    chunk_controller(allocator_type & allocator) :
    _allocator(allocator),
    _size(0),
    _last_chunk_remain(0)
  {}

  ~chunk_controller()
  {
    clear();
  }

  size_t write(const std::byte * buf, size_t len)
  {
    size_t orig_len = len;

    if (_last_chunk_remain != 0)
    {
      write_to_last(buf, len);
    }

    while (len > 0)
    {
      if (allocate_next() == false)
        break;
      write_to_last(buf, len);
    }

    return (orig_len - len);
  }

  size_t read_copy(size_t offset, std::byte * buf, size_t len)
  {
    const size_t orig_len = len;
    std::byte * read_buf = nullptr;

    // Read pointer first
    size_t rem = 0;
    while (len > 0)
    {
      rem = read(offset, read_buf, len);
      if (rem == 0)
        break;

      ::memcpy(buf, read_buf, rem);
      buf += rem;
      len -= rem;
      offset += rem;
    }

    return orig_len - len;
  }

  size_t read(size_t offset, std::byte *& buf, size_t len)
  {
    if (offset >= size() || _chunks.empty())
      return 0;

    //We pretend that there are consecutive chunks of memory
    //Also chunks MUST be always be the same size
    size_t chunk_size = _chunks.front().size();
    size_t chunk_id = offset / chunk_size;
    if (chunk_id >= _chunks.size())
      return 0;

    offset = offset % chunk_size;

    auto chunk = _chunks[chunk_id];
    if (offset >= chunk.size())
      return 0;

    buf = std::addressof(chunk.data()[offset]);
    if (chunk_id == _chunks.size() - 1 && _last_chunk_remain)
      chunk_size -= _last_chunk_remain;
    return std::min(chunk_size - offset, len);
  }

  void clear()
  {
    for (auto & it : _chunks)
      _allocator.deallocate(it);
    _size = 0;
    _last_chunk_remain = 0;
  }

  constexpr size_t size() const noexcept { return _size; }
  constexpr size_t remain() const noexcept { return _last_chunk_remain; }

private:
  allocator_type & _allocator;
  std::vector<chunk_type> _chunks;
  size_t _size;
  size_t _last_chunk_remain;

  void write_to_last(const std::byte *& buf, size_t & len)
  {
    auto last = _chunks.back();
    auto write_size = std::min(_last_chunk_remain, len);
    ::memcpy(last.data() + (last.size() - _last_chunk_remain), buf, write_size);
    buf += write_size;
    len -= write_size;
    _last_chunk_remain -= write_size;
  }

  bool allocate_next()
  {
    auto next = _allocator.allocate();
    if (next.empty())
      return false;
    _chunks.push_back(next);
    _last_chunk_remain = next.size();
    _size += next.size();
    return true;
  }
};

} // namespace ac

#endif // CHUNK_CONTROLLER_HPP
