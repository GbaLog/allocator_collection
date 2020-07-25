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

// The work of this class is to allocate and manipulate the blocks of memory.
// It provides easy interface to write and read blocks from it.

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
    _last_block_remain(0)
  {}

  ~chunk_controller()
  {
    clear();
  }

  size_t write(const std::byte * buf, size_t len)
  {
    size_t orig_len = len;

    if (_last_block_remain != 0)
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
    if (offset >= size() || _blocks.empty())
      return 0;

    //We pretend that there are consecutive blocks of memory
    //Also blocks MUST be always be the same size
    size_t block_size = _blocks.front().size();
    size_t block_id = offset / block_size;
    if (block_id >= _blocks.size())
      return 0;

    offset = offset % block_size;

    auto block = _blocks[block_id];
    if (offset >= block.size())
      return 0;

    buf = std::addressof(block.data()[offset]);
    return std::min(block.size() - offset, len);
  }

  void clear()
  {
    for (auto & it : _blocks)
      _allocator.deallocate(it);
  }

  constexpr size_t size() const noexcept { return _size; }
  constexpr size_t remain() const noexcept { return _last_block_remain; }

private:
  allocator_type & _allocator;
  std::vector<chunk_type> _blocks;
  size_t _size;
  size_t _last_block_remain;

  void write_to_last(const std::byte *& buf, size_t & len)
  {
    auto last = _blocks.back();
    auto write_size = std::min(_last_block_remain, len);
    ::memcpy(last.data() + (last.size() - _last_block_remain), buf, write_size);
    buf += write_size;
    len -= write_size;
    _last_block_remain -= write_size;
  }

  bool allocate_next()
  {
    auto next = _allocator.allocate();
    if (next.empty())
      return false;
    _blocks.push_back(next);
    _last_block_remain = next.size();
    _size += next.size();
    return true;
  }
};

} // namespace ac

#endif // CHUNK_CONTROLLER_HPP
