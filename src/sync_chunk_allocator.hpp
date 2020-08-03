#ifndef SYNC_CHUNK_ALLOCATOR_HPP
#define SYNC_CHUNK_ALLOCATOR_HPP

#include "ac_concepts.hpp"
#include <shared_mutex>

namespace ac
{

template<IsChunkAllocator Allocator>
class sync_chunk_allocator
{
public:
  using allocator_type = Allocator;
  using value_type = typename allocator_type::value_type;
  using chunk_type = typename allocator_type::chunk_type;

public:
  template<class ... Args>
  sync_chunk_allocator(Args &&... args) :
    _allocator{std::forward<Args>(args)...}
  {}

  [[nodiscard]]
  chunk_type allocate()
  {
    std::unique_lock lock(_mutex);
    return _allocator.allocate();
  }

  void deallocate(chunk_type chunk)
  {
    std::unique_lock lock(_mutex);
    _allocator.deallocate(chunk);
  }

  size_t size() const noexcept
  {
    std::shared_lock lock(_mutex);
    return _allocator.size();
  }

  size_t in_use() const noexcept
  {
    std::shared_lock lock(_mutex);
    return _allocator.in_use();
  }

  size_t remain() const noexcept
  {
    std::shared_lock lock(_mutex);
    return _allocator.remain();
  }

private:
  allocator_type _allocator;
  mutable std::shared_mutex _mutex;
};

} // namespace ac

#endif // SYNC_CHUNK_ALLOCATOR_HPP
