#include <gtest/gtest.h>
#include "block_allocator.hpp"

TEST(block_allocator, single_allocation)
{
  std::byte buf[1024] {};
  block_allocator_t allocator{buf, sizeof(buf), 512};

  auto block = allocator.allocate();
  EXPECT_EQ(512, block.size());
}

TEST(block_allocator, all_mem_exceed)
{
  std::byte buf[1024] {};
  block_allocator_t allocator{buf, sizeof(buf), 1024};

  auto block1 = allocator.allocate();
  auto block2 = allocator.allocate();
  EXPECT_EQ(1024, block1.size());
  EXPECT_TRUE(block2.empty());
}

TEST(block_allocator, with_deallocation)
{
  std::byte buf[1024] {};
  block_allocator_t allocator{buf, sizeof(buf), 1024};

  auto block = allocator.allocate();
  EXPECT_EQ(1024, block.size());

  allocator.deallocate(block);

  block = allocator.allocate();
  EXPECT_EQ(1024, block.size());
}

TEST(block_allocator, many_allocs_and_deallocs)
{
  std::byte buf[1024] {};
  block_allocator_t allocator{buf, sizeof(buf), 16};

  block_allocator_t::block_t blocks[64];

  for (auto & block : blocks)
  {
    block = allocator.allocate();
    EXPECT_EQ(16, block.size());
  }

  for (auto & block : blocks)
  {
    allocator.deallocate(block);
    block = block_allocator_t::block_t{};
  }

  for (auto & block : blocks)
  {
    block = allocator.allocate();
    EXPECT_EQ(16, block.size());
  }
}
