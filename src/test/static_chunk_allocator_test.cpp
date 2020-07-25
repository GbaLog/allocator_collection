#include <gtest/gtest.h>
#include "static_chunk_allocator.hpp"

TEST(static_chunk_allocator_test, single_allocation)
{
  std::byte buf[1024] {};
  ac::static_chunk_allocator allocator{buf, sizeof(buf), 512};

  ASSERT_EQ(2, allocator.size());

  auto chunk = allocator.allocate();
  EXPECT_EQ(512, chunk.size());
  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(1, allocator.remain());
}

TEST(static_chunk_allocator_test, all_mem_exceed)
{
  std::byte buf[1024] {};
  ac::static_chunk_allocator allocator{buf, sizeof(buf), 1024};

  ASSERT_EQ(1, allocator.size());

  auto chunk1 = allocator.allocate();
  auto chunk2 = allocator.allocate();
  EXPECT_EQ(1024, chunk1.size());
  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());
  EXPECT_TRUE(chunk2.empty());
}

TEST(static_chunk_allocator_test, with_deallocation)
{
  std::byte buf[1024] {};
  ac::static_chunk_allocator allocator{buf, sizeof(buf), 1024};

  ASSERT_EQ(1, allocator.size());

  auto chunk = allocator.allocate();
  EXPECT_EQ(1024, chunk.size());
  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  allocator.deallocate(chunk);
  EXPECT_EQ(0, allocator.in_use());
  EXPECT_EQ(1, allocator.remain());

  chunk = allocator.allocate();
  EXPECT_EQ(1024, chunk.size());
  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());
}

TEST(static_chunk_allocator_test, many_allocs_and_deallocs)
{
  std::byte buf[1024] {};
  ac::static_chunk_allocator allocator{buf, sizeof(buf), 16};

  ASSERT_EQ(64, allocator.size());

  ac::static_chunk_allocator::chunk_type chunks[64];

  for (auto & chunk : chunks)
  {
    chunk = allocator.allocate();
    EXPECT_EQ(16, chunk.size());
  }

  EXPECT_EQ(64, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  for (auto & chunk : chunks)
  {
    allocator.deallocate(chunk);
    chunk = ac::static_chunk_allocator::chunk_type{};
  }

  EXPECT_EQ(0, allocator.in_use());
  EXPECT_EQ(64, allocator.remain());

  for (auto & chunk : chunks)
  {
    chunk = allocator.allocate();
    EXPECT_EQ(16, chunk.size());
  }

  EXPECT_EQ(64, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());
}

TEST(static_chunk_allocator_test, deallocate_wrong_pointer)
{
  std::byte buf[1024] {};
  ac::static_chunk_allocator allocator{buf, sizeof(buf), 1024};

  ASSERT_EQ(1, allocator.size());

  auto chunk = allocator.allocate();

  EXPECT_EQ(1024, chunk.size());
  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  auto wrong_chunk = chunk.subspan(1);

  allocator.deallocate(wrong_chunk);

  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  wrong_chunk.subspan(10000);

  allocator.deallocate(wrong_chunk);

  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  wrong_chunk = ac::static_chunk_allocator::chunk_type{};

  allocator.deallocate(wrong_chunk);

  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  // Check what we don't break anything
  allocator.deallocate(chunk);

  EXPECT_EQ(0, allocator.in_use());
  EXPECT_EQ(1, allocator.remain());
}
