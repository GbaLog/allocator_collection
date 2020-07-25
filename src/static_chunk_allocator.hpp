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
  static_chunk_allocator(value_type * buf, size_t buf_len, size_t chunk_size) :
    _buf{buf, buf_len}, _chunk_size{chunk_size},
    _chunks_count{_buf.size_bytes() / _chunk_size}
  {
    assert((buf_len % chunk_size) == 0 && "There MUSTN'T be the remainder");
    slice_to_chunks();
  }

  chunk_type allocate()
  {
    if (_unused_chunk_ids.empty())
      return {};

    auto chunk_id = _unused_chunk_ids.front();
    _unused_chunk_ids.pop_front();
    return _buf.subspan(_chunk_size * chunk_id, _chunk_size);
  }

  void deallocate(chunk_type chunk)
  {
    size_t chunk_place = chunk.data() - _buf.data();
    if (chunk_place % _chunk_size != 0)
      return;
    size_t chunk_id = chunk_place / _chunk_size;
    if (chunk_id >= _chunks_count)
      return;

    _unused_chunk_ids.push_back(chunk_place / _chunk_size);
  }

  size_t size()   const noexcept { return _chunks_count; }
  size_t remain() const noexcept { return _unused_chunk_ids.size(); }
  size_t in_use() const noexcept { return size() - remain(); }

private:
  chunk_type _buf;
  const size_t _chunk_size;
  const size_t _chunks_count;
  std::deque<uint32_t> _unused_chunk_ids;

  void slice_to_chunks()
  {
    // If buffer_size divided by chunk_size has a remainder, then last chunk will be discarded
    // It's important because all chunks in this allocator MUST be the same size
    size_t chunk_count = _buf.size_bytes() / _chunk_size;

    _unused_chunk_ids.resize(chunk_count);
    std::iota(_unused_chunk_ids.begin(), _unused_chunk_ids.end(), 0);
  }
};

} // namespace ac

#endif // STATIC_CHUNK_ALLOCATOR_HPP
