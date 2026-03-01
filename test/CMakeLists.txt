cmake_minimum_required (VERSION 3.18.1)

set (TARGET_NAME kingfisher_base_test)

include_directories (.)
aux_source_directory (base SRCS)
aux_source_directory (proto SRCS)
aux_source_directory (pkg SRCS)

add_executable (${TARGET_NAME} ${SRCS})

# 使用 gtest_discover_tests 自动发现所有 gtest 用例并独立注册到 ctest
# 每个测试用例有独立的超时控制（默认30秒）
include(GoogleTest)
gtest_discover_tests(${TARGET_NAME}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  PROPERTIES TIMEOUT 30
)
#add_definitions ("-std=c++0x -Wno-deprecated -D_GLIBCXX_USE_NANOSLEEP")
target_link_libraries (${TARGET_NAME} 
  kingfisher_base 
  kingfisher_pkg
  proto-image 
  gtest 
  ${OPENCV_DEPS} 
  ${MAGICK_DEPS}
  ${PROTOBUF_DEPS}
  # ${GRPC_DEPS} - 暂时移除，避免 protobuf 版本冲突（grpc 内置 3.12.2 vs 项目 3.6.1）
  pthread 
  dl
  z
  rt)
