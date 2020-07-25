#ifndef AC_CONCEPTS_HPP
#define AC_CONCEPTS_HPP

#include <concepts>

namespace ac
{

template<class T>
concept IsChunkAllocator = requires(T & val)
{
  { val.allocate() } -> std::same_as<typename T::chunk_t>;
  val.deallocate(typename T::chunk_t{});
  { val.remain() } -> std::same_as<size_t>;
  { val.in_use() } -> std::same_as<size_t>;
  { val.size() } -> std::same_as<size_t>;
};

}

#endif // AC_CONCEPTS_HPP
