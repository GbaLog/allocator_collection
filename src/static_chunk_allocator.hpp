#ifndef STATIC_CHUNK_ALLOCATOR_HPP
#define STATIC_CHUNK_ALLOCATOR_HPP

#include <deque>
#include <cstddef>
#include <numeric>
#include <span>
#include <cassert>

namespace ac
{

class static_chunk_allocator
{
public:
  using value_type = std::byte;
  using chunk_type = std::span<value_type>;

public:
  static_chunk_allocator(value_type * buf, size_t buf_len, size_t block_size) :
    _buf{buf, buf_len}, _block_size{block_size},
    _blocks_count{_buf.size_bytes() / _block_size}
  {
    assert((buf_len % block_size) == 0 && "There MUSTN'T be the remainder");
    slice_to_blocks();
  }

  chunk_type allocate()
  {
    if (_unused_block_ids.empty())
      return {};

    auto block_id = _unused_block_ids.front();
    _unused_block_ids.pop_front();
    return _buf.subspan(_block_size * block_id, _block_size);
  }

  void deallocate(chunk_type chunk)
  {
    size_t chunk_place = chunk.data() - _buf.data();
    if (chunk_place % _block_size != 0)
      return;
    size_t block_id = chunk_place / _block_size;
    if (block_id >= _blocks_count)
      return;

    _unused_block_ids.push_back(chunk_place / _block_size);
  }

  size_t size()   const noexcept { return _blocks_count; }
  size_t remain() const noexcept { return _unused_block_ids.size(); }
  size_t in_use() const noexcept { return size() - remain(); }

private:
  chunk_type _buf;
  const size_t _block_size;
  const size_t _blocks_count;
  std::deque<uint32_t> _unused_block_ids;

  void slice_to_blocks()
  {
    // If buffer_size divided by block_size has a remainder, then last block will be discarded
    // It's important because all blocks in this allocator MUST be the same size
    size_t block_count = _buf.size_bytes() / _block_size;

    _unused_block_ids.resize(block_count);
    std::iota(_unused_block_ids.begin(), _unused_block_ids.end(), 0);
  }
};

} // namespace ac

#endif // STATIC_CHUNK_ALLOCATOR_HPP
