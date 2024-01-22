cmake_minimum_required (VERSION 3.18.1)

project (kingfisher CXX)

# ignore warning
# add_definitions(-w)

#if(NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Debug")
#endif()

set(CXX_FLAGS
 -g
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
 # -Wconversion
 -Wno-unused-parameter
 -Wno-unused-local-typedefs
 -Wno-missing-field-initializers
 # -Wold-style-cast
 # -Woverloaded-virtual
 -Wpointer-arith
 -Wno-noexcept-type
 -Waligned-new
 # -Wshadow
 -Wwrite-strings
 -march=native
 -std=c++17
 -rdynamic
 )

# generate compile_commands.json
set (CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set (CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -D__STDC_FORMAT_MACROS -fprofile-arcs -ftest-coverage -fPIC")
set (CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -finline-limit=1000 -DNDEBUG -D__STDC_FORMAT_MACROS")
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set (CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/../output)
set (INCLUDE_OUTPUL_PATH ${CMAKE_INSTALL_PREFIX}/include)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
#set_target_properties(kingfisher_base PROPERTIES PUBLIC_HEADER "*.h")

# 查找find_package
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
message(STATUS "CMAKE_MODULE_PATH = " ${CMAKE_MODULE_PATH})

string (TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

include_directories (.)
include_directories (base)
include_directories (pkg)
include_directories (third_party)
include(cmake/Build.options.cmake)


add_subdirectory (base)
add_subdirectory (pkg)

link_directories(${CMAKE_INSTALL_PREFIX}/lib)
add_subdirectory (test)

install(TARGETS kingfisher_base LIBRARY DESTINATION ${LIBRARY_OUTPUT_PATH})
message(STATUS "> install lib target ${LIBRARY_OUTPUT_PATH}")

#INSTALL(TARGETS kingfisher_base 
#        LIBRARY DESTINATION ${LIBRARY_OUTPUT_PATH}
#        PUBLIC_HEADER DESTINATION ${PROJECT_BINARY_DIR}/include
#)
