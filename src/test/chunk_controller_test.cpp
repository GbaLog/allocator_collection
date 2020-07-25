#include <gtest/gtest.h>
#include "static_chunk_allocator.hpp"
#include "chunk_controller.hpp"

class chunk_controller_test : public ::testing::Test
{
public:
  virtual void SetUp() override
  {
    _alloc = new ac::static_chunk_allocator{_buf, sizeof(_buf), 16};
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
