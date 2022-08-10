# find FindProtobuf.cmake
find_package(Protobuf)
GET_MODEL_DIRS(${CMAKE_CURRENT_SOURCE_DIR} _module_dirs)
FOREACH (_module_dir ${_module_dirs})
    if (EXISTS ${_module_dir})
      file(RELATIVE_PATH _rel_module_dir ${CMAKE_CURRENT_SOURCE_DIR} ${_module_dir})
      message(STATUS "add_subdirectory=${_rel_module_dir}")
      add_subdirectory(${_rel_module_dir})
      # add_dependencies(openapi-spec proto-${_rel_module_dir})
    endif()
ENDFOREACH ()

file(GLOB_RECURSE PKG_SRC TRUE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
list(FILTER PKG_SRC EXCLUDE REGEX "main.*")
message(STATUS "PKG_SRC = ${PKG_SRC}")

#add_library(kingfisher_pkg SHARED ${PKG_SRC})
add_library(kingfisher_pkg STATIC ${PKG_SRC})
file(GLOB_RECURSE ALLEGRO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
target_link_libraries(kingfisher_pkg ${OPENCV_DEPS} ${MAGICK_DEPS} ${PROTOBUF_DEPS})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${INCLUDE_OUTPUL_PATH}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp")


