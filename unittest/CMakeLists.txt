cmake_minimum_required(VERSION 2.8)

project(unittest)

find_package (GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB_RECURSE SRCS_UNITTEST *.c*)
list(APPEND SRCS_UNITTEST ${CMAKE_CURRENT_SOURCE_DIR}/../test/asio.cc)

add_executable(unittest ${SRCS_UNITTEST})
TARGET_LINK_LIBRARIES(unittest mprpc-static
    ${GTEST_LIBRARIES}
)
