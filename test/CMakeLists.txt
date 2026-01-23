cmake_minimum_required (VERSION 3.18.1)

set (TARGET_NAME kingfisher_base_test)

include_directories (.)
aux_source_directory (base SRCS)
aux_source_directory (proto SRCS)
aux_source_directory (pkg SRCS)

add_executable (${TARGET_NAME} ${SRCS})
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
