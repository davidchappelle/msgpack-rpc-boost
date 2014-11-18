find_package (GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB_RECURSE SRCS_UNITTEST *.c*)
list(APPEND SRCS_UNITTEST ${CMAKE_CURRENT_SOURCE_DIR}/../test/asio.cc)

set(MSGPACK_RPC_LIBRARY mprpc)

add_executable(unittest ${SRCS_UNITTEST})
add_dependencies(sync_call ${MSGPACK_RPC_LIBRARY})
target_link_libraries(unittest mprpc ${GTEST_LIBRARIES})
