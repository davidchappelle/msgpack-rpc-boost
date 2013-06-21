project(unittest)
cmake_minimum_required(VERSION 2.6)

# Set paths for required thirdparty libraries.
set(CLIENT_GOOGLE_TEST_CPP ../../../thirdparty/gtest-1.5.0)

find_library(GTEST gtest HINTS ${CLIENT_GOOGLE_TEST_CPP}/build)
find_library(GTEST_MAIN gtest_main HINTS ${CLIENT_GOOGLE_TEST_CPP}/build)
include_directories(${CLIENT_GOOGLE_TEST_CPP}/include/)

find_library(LIBMSGPACK NAMES "libmsgpack.a" "msgpack" PATHS "../../lib")
find_library(GLOG_LIBRARIES NAMES "glog" PATHS "../../lib")

file(GLOB_RECURSE SRCS_UNITTEST *.c*)
add_executable(unittest ${SRCS_UNITTEST})
TARGET_LINK_LIBRARIES(unittest mprpc ${LIBMSGPACK}
    ${GLOG_LIBRARIES} ${GTEST_MAIN} ${GTEST}
    ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${LIBPTHREAD}
    ${Boost_PROGRAM_OPTIONS_LIBRARY} ${OPENSSL_LIBRARIES}
)
