file(GLOB_RECURSE PKG_SRC TRUE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
list(FILTER PKG_SRC EXCLUDE REGEX "main.*")
message(STATUS "PKG_SRC = ${PKG_SRC}")

#add_library(kingfisher_pkg SHARED ${PKG_SRC})
add_library(kingfisher_pkg STATIC ${PKG_SRC})
file(GLOB_RECURSE ALLEGRO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
target_link_libraries(kingfisher_pkg ${OPENCV_DEPS} ${MAGICK_DEPS} ${PROTOBUF_DEPS})
