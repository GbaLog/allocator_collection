#include <gtest/gtest.h>
#include "static_chunk_allocator.hpp"

TEST(static_chunk_allocator_test, single_allocation)
{
  std::byte buf[1024] {};
  ac::static_chunk_allocator allocator{buf, sizeof(buf), 512};

  ASSERT_EQ(2, allocator.size());

  auto block = allocator.allocate();
  EXPECT_EQ(512, block.size());
  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(1, allocator.remain());
}

TEST(static_chunk_allocator_test, all_mem_exceed)
{
  std::byte buf[1024] {};
  ac::static_chunk_allocator allocator{buf, sizeof(buf), 1024};

  ASSERT_EQ(1, allocator.size());

  auto block1 = allocator.allocate();
  auto block2 = allocator.allocate();
  EXPECT_EQ(1024, block1.size());
  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());
  EXPECT_TRUE(block2.empty());
}

TEST(static_chunk_allocator_test, with_deallocation)
{
  std::byte buf[1024] {};
  ac::static_chunk_allocator allocator{buf, sizeof(buf), 1024};

  ASSERT_EQ(1, allocator.size());

  auto block = allocator.allocate();
  EXPECT_EQ(1024, block.size());
  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  allocator.deallocate(block);
  EXPECT_EQ(0, allocator.in_use());
  EXPECT_EQ(1, allocator.remain());

  block = allocator.allocate();
  EXPECT_EQ(1024, block.size());
  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());
}

TEST(static_chunk_allocator_test, many_allocs_and_deallocs)
{
  std::byte buf[1024] {};
  ac::static_chunk_allocator allocator{buf, sizeof(buf), 16};

  ASSERT_EQ(64, allocator.size());

  ac::static_chunk_allocator::chunk_t blocks[64];

  for (auto & block : blocks)
  {
    block = allocator.allocate();
    EXPECT_EQ(16, block.size());
  }

  EXPECT_EQ(64, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  for (auto & block : blocks)
  {
    allocator.deallocate(block);
    block = ac::static_chunk_allocator::chunk_t{};
  }

  EXPECT_EQ(0, allocator.in_use());
  EXPECT_EQ(64, allocator.remain());

  for (auto & block : blocks)
  {
    block = allocator.allocate();
    EXPECT_EQ(16, block.size());
  }

  EXPECT_EQ(64, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());
}

TEST(static_chunk_allocator_test, deallocate_wrong_pointer)
{
  std::byte buf[1024] {};
  ac::static_chunk_allocator allocator{buf, sizeof(buf), 1024};

  ASSERT_EQ(1, allocator.size());

  auto block = allocator.allocate();

  EXPECT_EQ(1024, block.size());
  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  auto wrong_block = block.subspan(1);

  allocator.deallocate(wrong_block);

  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  wrong_block.subspan(10000);

  allocator.deallocate(wrong_block);

  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  wrong_block = ac::static_chunk_allocator::chunk_t{};

  allocator.deallocate(wrong_block);

  EXPECT_EQ(1, allocator.in_use());
  EXPECT_EQ(0, allocator.remain());

  // Check what we don't break anything
  allocator.deallocate(block);

  EXPECT_EQ(0, allocator.in_use());
  EXPECT_EQ(1, allocator.remain());
}
