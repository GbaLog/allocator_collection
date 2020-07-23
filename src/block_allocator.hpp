#ifndef BLOCK_ALLOCATOR_H
#define BLOCK_ALLOCATOR_H

#include <deque>
#include <cstddef>
#include <numeric>
#include <span>
#include <cassert>

class block_allocator_t
{
public:
  using block_t = std::span<std::byte>;

public:
  block_allocator_t(std::byte * buf, size_t buf_len, size_t block_size) :
    _buf{buf, buf_len}, _block_size{block_size}
  {
    assert((buf_len % block_size) == 0 && "There MUSTN'T be the remainder");
    slice_to_blocks();
  }

  block_t allocate();
  void deallocate(block_t);

public:
  block_t _buf;
  const size_t _block_size;
  std::deque<uint32_t> _unused_block_ids;

  void slice_to_blocks();
};

block_allocator_t::block_t block_allocator_t::allocate()
{
  if (_unused_block_ids.empty())
    return {};

  auto block_id = _unused_block_ids.front();
  _unused_block_ids.pop_front();
  return _buf.subspan(_block_size * block_id, _block_size);
}

void block_allocator_t::deallocate(block_t block)
{
  _unused_block_ids.push_back((block.data() - _buf.data()) / _block_size);
}

void block_allocator_t::slice_to_blocks()
{
  // If buffer_size divided by block_size has a remainder, then last block will be discarded
  // It's important because all blocks in this allocator MUST be the same size
  size_t block_count = _buf.size_bytes() / _block_size;

  _unused_block_ids.resize(block_count);
  std::iota(_unused_block_ids.begin(), _unused_block_ids.end(), 0);
}

#endif // BLOCK_ALLOCATOR_H
