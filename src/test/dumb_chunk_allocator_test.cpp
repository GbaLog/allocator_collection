#include <gtest/gtest.h>
#include "dumb_chunk_allocator.hpp"

TEST(dumb_chunk_allocator_test, single_allocate)
{
  ac::dumb_chunk_allocator alloc{16, 10};

  // It allocates blocks only if needed
  ASSERT_EQ(0, alloc.size());

  auto chunk = alloc.allocate();
  EXPECT_FALSE(chunk.empty());

  EXPECT_EQ(1, alloc.size());
  EXPECT_EQ(1, alloc.in_use());
  EXPECT_EQ(0, alloc.remain());
}

TEST(dumb_chunk_allocator_test, no_more_blocks_avaiable)
{
  ac::dumb_chunk_allocator alloc{16, 1};

  auto chunk = alloc.allocate();
  EXPECT_FALSE(chunk.empty());
  EXPECT_EQ(16, chunk.size());

  auto chunk2 = alloc.allocate();
  EXPECT_TRUE(chunk2.empty());
}

TEST(dumb_chunk_allocator_test, no_blocks_avaiable_at_all)
{
  ac::dumb_chunk_allocator alloc{16, 0};
  auto chunk = alloc.allocate();
  EXPECT_TRUE(chunk.empty());
}

TEST(dumb_chunk_allocator_test, chunk_reuse)
{
  ac::dumb_chunk_allocator alloc{16, 1};
  auto chunk = alloc.allocate();
  EXPECT_FALSE(chunk.empty());
  EXPECT_EQ(16, chunk.size());

  alloc.deallocate(chunk);

  auto chunk2 = alloc.allocate();
  EXPECT_FALSE(chunk2.empty());
  EXPECT_EQ(16, chunk2.size());
}

TEST(dumb_chunk_allocator_test, chunk_reusing_may_chunks)
{
  ac::dumb_chunk_allocator alloc{16, 10};

  ac::dumb_chunk_allocator::chunk_type chunks[10];
  for (auto & it : chunks)
  {
    it = alloc.allocate();
    EXPECT_FALSE(it.empty());
    EXPECT_EQ(16, it.size());
  }

  EXPECT_EQ(10, alloc.size());
  EXPECT_EQ(10, alloc.in_use());
  EXPECT_EQ(0, alloc.remain());

  for (auto & it : chunks)
    alloc.deallocate(it);

  EXPECT_EQ(10, alloc.size());
  EXPECT_EQ(0, alloc.in_use());
  EXPECT_EQ(10, alloc.remain());

  for (auto & it : chunks)
  {
    it = alloc.allocate();
    EXPECT_FALSE(it.empty());
    EXPECT_EQ(16, it.size());
  }
}

TEST(dumb_chunk_allocator_test, dealloc_wrong_chunk)
{
  ac::dumb_chunk_allocator alloc{16, 1};
  auto chunk = alloc.allocate();
  EXPECT_FALSE(chunk.empty());
  EXPECT_EQ(16, chunk.size());

  EXPECT_EQ(1, alloc.size());
  EXPECT_EQ(1, alloc.in_use());
  EXPECT_EQ(0, alloc.remain());

  auto broken = chunk.subspan(1);
  alloc.deallocate(broken);

  EXPECT_EQ(1, alloc.size());
  EXPECT_EQ(1, alloc.in_use());
  EXPECT_EQ(0, alloc.remain());

  broken = chunk.subspan(10000);
  alloc.deallocate(broken);

  EXPECT_EQ(1, alloc.size());
  EXPECT_EQ(1, alloc.in_use());
  EXPECT_EQ(0, alloc.remain());
}
