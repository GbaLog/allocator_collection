#include <gtest/gtest.h>
#include "static_chunk_allocator.hpp"
#include "chunk_controller.hpp"

class chunk_controller_test : public ::testing::Test
{
public:
  virtual void SetUp() override
  {
    _alloc = new ac::static_chunk_allocator{_buf, sizeof(_buf), 512};
  }

  virtual void TearDown() override
  {
    delete _alloc;
    ::memset(_buf, 0x00, sizeof(_buf));
  }

protected:
  std::byte _buf[1024];
  ac::static_chunk_allocator * _alloc;
};

TEST_F(chunk_controller_test, overwrite)
{
  ac::chunk_controller ctl{*_alloc};

  std::byte buf[3333] {};
  size_t written = ctl.write(buf, sizeof(buf));
  EXPECT_EQ(1024, written);
}

TEST_F(chunk_controller_test, two_allocations)
{
  ac::chunk_controller ctl{*_alloc};

  std::byte buf[33] {};
  std::byte buf2[479] {};

  size_t written = ctl.write(buf, sizeof(buf));
  EXPECT_EQ(33, written);

  written = ctl.write(buf2, sizeof(buf2));
  EXPECT_EQ(479, written);

  // Controller musn't allocate blocks until it's needed
  EXPECT_EQ(1, _alloc->remain());
}

TEST_F(chunk_controller_test, consequent_allocations)
{
  ac::chunk_controller ctl{*_alloc};

  std::byte buf[33] {};
  std::byte buf2[479] {};

  size_t written = ctl.write(buf, sizeof(buf));
  EXPECT_EQ(33, written);

  written = ctl.write(buf2, sizeof(buf2));
  EXPECT_EQ(479, written);

  // Controller musn't allocate blocks until it's needed
  EXPECT_EQ(1, _alloc->remain());

  written = ctl.write(buf, 1);
  EXPECT_EQ(1, written);

  // Now it should have allocate remaining block
  EXPECT_EQ(0, _alloc->remain());

  std::byte buf3[512] {};
  written = ctl.write(buf3, sizeof(buf3));
  // First byte of last allocated block is busy by 1 byte from last write
  // And there's no available blocks, so it writes as much as it can
  EXPECT_EQ(511, written);
}

TEST_F(chunk_controller_test, read_from_begin_with_remaining_size)
{
  ac::chunk_controller ctl{*_alloc};

  std::byte buf[33] {};
  size_t written = ctl.write(buf, sizeof(buf));
  EXPECT_EQ(33, written);

  std::byte * to_read = nullptr;
  // Must return 33, because only 33 bytes was written in first block
  size_t read = ctl.read(0, to_read, 33);
  EXPECT_EQ(33, read);

  // Less than was written
  read = ctl.read(0, to_read, 30);
  EXPECT_EQ(30, read);

  // With offset
  read = ctl.read(10, to_read, 33);
  EXPECT_EQ(23, read);
}

TEST_F(chunk_controller_test, read_from_begin_without_remaining_size)
{
  ac::chunk_controller ctl{*_alloc};

  std::byte buf[1024] {};
  size_t written = ctl.write(buf, sizeof(buf));
  EXPECT_EQ(1024, written);

  std::byte * to_read = nullptr;
  size_t read = ctl.read(0, to_read, 1024);
  // Because read() function read up to the chunk boundary
  // We should use read_copy() instead
  EXPECT_EQ(512, read);
}

TEST_F(chunk_controller_test, read_in_the_middle_of_chunk)
{
  ac::chunk_controller ctl{*_alloc};

  std::byte buf[1024] {};
  buf[256] = (std::byte)0x01;
  buf[767] = (std::byte)0x02;
  size_t written = ctl.write(buf, sizeof(buf));
  EXPECT_EQ(1024, written);

  EXPECT_EQ(2, _alloc->in_use());

  std::byte * to_read = nullptr;
  size_t read = ctl.read(256, to_read, 512);
  EXPECT_EQ(256, read);
  EXPECT_EQ((std::byte)0x01, to_read[0]);

  std::byte copy_buf[512] {};
  read = ctl.read_copy(256, copy_buf, sizeof(copy_buf));
  EXPECT_EQ(512, read);

  EXPECT_EQ((std::byte)0x01, copy_buf[0]);
  EXPECT_EQ((std::byte)0x02, copy_buf[511]);
}

TEST_F(chunk_controller_test, read_in_the_end_of_last_chunk)
{
  ac::chunk_controller ctl{*_alloc};

  std::byte buf[512] {};
  std::byte buf2[512] {};
  buf2[0]   = (std::byte)0x03;
  buf2[511] = (std::byte)0x04;
  size_t written = ctl.write(buf, sizeof(buf));
  EXPECT_EQ(512, written);
  written = ctl.write(buf2, sizeof(buf2));
  EXPECT_EQ(512, written);

  EXPECT_EQ(2, _alloc->in_use());

  std::byte * to_read = nullptr;
  size_t read = ctl.read(512, to_read, 10);
  EXPECT_EQ(10, read);
  EXPECT_EQ((std::byte)0x03, to_read[0]);

  std::byte copy_buf[512] {};
  read = ctl.read_copy(512, copy_buf, sizeof(copy_buf));
  EXPECT_EQ(512, read);

  EXPECT_EQ((std::byte)0x03, copy_buf[0]);
  EXPECT_EQ((std::byte)0x04, copy_buf[511]);
}
