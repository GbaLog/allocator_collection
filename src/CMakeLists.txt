cmake_minimum_required(VERSION 3.10)

project(block_allocator_test CXX)

enable_testing()
find_package(GTest)

add_executable(${PROJECT_NAME}
  test/static_chunk_allocator_test.cpp
  test/chunk_controller_test.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON)

include_directories("${PROJECT_SOURCE_DIR}")

if (GTEST_FOUND)
  message(STATUS "Link to GTest system library")
  target_link_libraries(${PROJECT_NAME} GTest::GTest GTest::Main)
else (GTEST_FOUND)
  message(STATUS "Compile googletest from source in submodule")
  add_subdirectory(googletest)
  include_directories("${PROJECT_SOURCE_DIR}/googletest/googletest/include")

  target_link_libraries(${PROJECT_NAME} gtest gtest_main)
endif (GTEST_FOUND)

add_test(test ${PROJECT_NAME})
