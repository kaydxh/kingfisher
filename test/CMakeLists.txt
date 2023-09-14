cmake_minimum_required (VERSION 3.18.1)

set (TARGET_NAME kingfisher_base_test)

include_directories (.)
aux_source_directory (base SRCS)
aux_source_directory (proto SRCS)
aux_source_directory (pkg SRCS)

add_executable (${TARGET_NAME} ${SRCS})
#add_definitions ("-std=c++0x -Wno-deprecated -D_GLIBCXX_USE_NANOSLEEP")
target_link_libraries (${TARGET_NAME} kingfisher_base kingfisher_pkg
  gtest proto-image pthread)
