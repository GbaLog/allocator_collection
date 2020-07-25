#ifndef DUMB_CHUNK_ALLOCATOR_HPP
#define DUMB_CHUNK_ALLOCATOR_HPP

#include <cstddef>
#include <span>
#include <deque>
#include <memory>

namespace ac
{

namespace detail
{

template<class Value>
struct dumb_chunk
{
  using value_type = Value;
  using chunk_type = std::span<value_type>;

  std::unique_ptr<value_type []> _ptr;
  chunk_type _chunk;
};

template<class T>
constexpr bool operator ==(const dumb_chunk<T> & lhs,
                           const typename dumb_chunk<T>::chunk_type & rhs)
{
  return lhs._chunk.data() == rhs.data();
}

}

class dumb_chunk_allocator
{
public:
  using value_type = std::byte;
  using chunk_type = std::span<std::byte>;

public:
  dumb_chunk_allocator(size_t chunk_size, size_t max_chunks) :
    _chunk_size{chunk_size},
    _max_chunks{max_chunks}
  {}

  chunk_type allocate()
  {
    if (_free_chunks.empty())
    {
      if (size() >= _max_chunks)
        return chunk_type{};
      allocate_next();
    }
    auto ret = _free_chunks.front();
    _free_chunks.pop_front();
    return ret;
  }

  void deallocate(chunk_type chunk)
  {
    auto it = std::find(_all_chunks.begin(), _all_chunks.end(), chunk);
    if (it == _all_chunks.end())
      return;
    _free_chunks.push_back(chunk);
  }

  size_t size()   const noexcept { return _all_chunks.size(); }
  size_t remain() const noexcept { return _free_chunks.size(); }
  size_t in_use() const noexcept { return _all_chunks.size() - remain(); }

private:
  using internal_chunk = detail::dumb_chunk<value_type>;

  const size_t _chunk_size;
  const size_t _max_chunks;
  std::deque<chunk_type> _free_chunks;
  std::deque<internal_chunk> _all_chunks;

  void allocate_next()
  {
    internal_chunk new_chunk
      { ._ptr = std::make_unique<value_type []>(_chunk_size) };
    new_chunk._chunk = chunk_type{new_chunk._ptr.get(), _chunk_size};

    _all_chunks.push_back(std::move(new_chunk));
    _free_chunks.push_back(new_chunk._chunk);
  }
};

}

#endif // DUMB_CHUNK_ALLOCATOR_HPP
