file(GLOB_RECURSE BASE_SRC TRUE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
list(FILTER BASE_SRC EXCLUDE REGEX "main.*")
# Exclude grpc files if grpc is not available
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/grpc/include)
  list(FILTER BASE_SRC EXCLUDE REGEX ".*grpc.*")
endif()
# Exclude curl files if curl is not available
if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/curl/include)
  list(FILTER BASE_SRC EXCLUDE REGEX ".*curl.*")
  list(FILTER BASE_SRC EXCLUDE REGEX ".*http_client.*")
endif()
message(STATUS "BASE_SRC = ${BASE_SRC}")


add_library(kingfisher_base SHARED ${BASE_SRC})
file(GLOB_RECURSE ALLEGRO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
target_link_libraries(kingfisher_base ${OPENSSL_DEPS} ${LIBCO_DEPS})
# message(STATUS "current dir = ${CMAKE_CURRENT_SOURCE_DIR}, include files =  ${ALLEGRO_INCLUDE_DIR}")

#set_target_properties(kingfisher_base PROPERTIES PUBLIC_HEADER "${ALLEGRO_INCLUDE_DIR}")
message(STATUS "dir = ${CMAKE_CURRENT_BINARY_DIR}")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION ${INCLUDE_OUTPUL_PATH}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp")


