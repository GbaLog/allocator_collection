#ifdef __linux__

#ifndef SHARED_MEMORY_ALLOCATOR_HPP
#define SHARED_MEMORY_ALLOCATOR_HPP

#include <sys/ipc.h>
#include <sys/shm.h>
#include <cerrno>
#include <optional>

namespace ac
{

class shared_memory_allocator
{
public:
  shared_memory_allocator(key_t key, size_t size, int flags) :
    _key{key},
    _size{size},
    _flags{flags}
  {}

  ~shared_memory_allocator()
  {
    if (attached())
      detach();
  }

  [[nodiscard]]
  bool attach() noexcept
  {
    if (_segment_id == false)
      return false;

    int segment_id = *_segment_id;
    void * base_address = shmat(segment_id, 0, 0);
    if (base_address == (void *)-1)
      return false;

    _base_address = base_address;
    return true;
  }

  [[nodiscard]]
  bool detach() noexcept
  {
    if (_base_address == false)
      return false;
    int res = shmdt(*_base_address);
    if (res == 0)
    {
      _segment_id.reset();
      _base_address.reset();
    }
    return res == 0;
  }

  [[nodiscard]]
  bool allocate() noexcept
  {
    // Try to allocate new block
    int segment_id = shmget(_key, _size, _flags);
    if (segment_id != -1)
    {
      _segment_id = segment_id;
      return true;
    }

    // If already exist, try to attach
    if (errno == EEXIST)
    {
      segment_id = shmget(_ket, 0, 0);
      // Attach failed :(
      if (segment_id == -1)
        return false;

      _segment_id = segment_id;
      return true;
    }
    return false;
  }

  [[nodiscard]]
  bool attached() const noexcept
  {
    return _base_address.has_value();
  }

  [[nodiscard]]
  bool attach_or_allocate() noexcept
  {
    return attach() || (allocate() && attach());
  }

  [[nodiscard]]
  std::optional<void *> get_base_pointer() const noexcept
  {
    return _base_address;
  }

private:
  key_t _key;
  size_t _size;
  int _flags;
  std::optional<int> _segment_id;
  std::optional<void *> _base_address;
};

}

#endif // SHARED_MEMORY_ALLOCATOR_HPP

#endif // __linux__
