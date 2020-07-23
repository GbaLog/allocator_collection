cmake_minimum_required(VERSION 3.10)

project(block_allocator_test CXX)

enable_testing()

add_executable(${PROJECT_NAME}
  block_allocator_test.cpp
  block_entry_test.cpp)

find_package(GTest)

if (GTEST_FOUND)
  target_link_libraries(${PROJECT_NAME} GTest::GTest GTest::Main)
else (GTEST_FOUND)
  add_subdirectory(googletest)
  include_directories("${PROJECT_SOURCE_DIR}/googletest/googletest/include")

  target_link_libraries(${PROJECT_NAME} GTest::GTest GTest::Main)
endif (GTEST_FOUND)

add_test(test ${PROJECT_NAME})