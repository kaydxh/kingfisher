

#set (
#  base_SRC
#  configparser/config_parser.cc
#  dump/stack_trace.cc
#  file/file.cc
#  file/file_util.cc
#  crypto/md5.cc
#  crypto/sha256.cc
#  net/ip.cc
#  os/getwd.cc
#  os/signal/signal.cc
#  path/path.cc
#  serializer/djson.cc
#  strings/strings.cc
#  thread/thread.cc
#  thread/thread_pool.cc
#  time/date.cc
#  time/timer.cc
#  time/time_counter.cc
#  time/timestamp.cc
#  time/time_zone.cc
#  uuid/guid.cc
#  )
#
file(GLOB_RECURSE BASE_SRC TRUE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
list(FILTER BASE_SRC EXCLUDE REGEX "main.*")
message(STATUS "BASE_SRC= ${BASE_SRC}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/configparser)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/crypto)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/serializer)

add_library(kingfisher_base SHARED ${BASE_SRC})
file(GLOB_RECURSE ALLEGRO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
target_link_libraries(kingfisher_base ${OPENSSL_DEPS})
# message(STATUS "current dir = ${CMAKE_CURRENT_SOURCE_DIR}, include files =  ${ALLEGRO_INCLUDE_DIR}")

#set_target_properties(kingfisher_base PROPERTIES PUBLIC_HEADER "${ALLEGRO_INCLUDE_DIR}")
message(STATUS "dir= ${CMAKE_CURRENT_BINARY_DIR}")
#install(TARGETS kingfisher_base DESTINATION lib)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/../output/include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        )

install(TARGETS kingfisher_base DESTINATION output/lib)

#INSTALL(TARGETS kingfisher_base 
#        LIBRARY DESTINATION lib 
        #        PUBLIC_HEADER DESTINATION include
        #)

