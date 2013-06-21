cmake_minimum_required(VERSION 2.6)
PROJECT(test)

set(CMAKE_CXX_FLAGS "-g -Wall")

find_library(LIBMSGPACK NAMES "libmsgpack.a" "msgpack" PATHS "../../lib")
find_library(GLOG_LIBRARIES NAMES "glog" PATHS "../../lib")
set(LIBMPRPC "${CMAKE_BINARY_DIR}/src/msgpack/rpc/libmprpc.a")

link_libraries(${LIBMPRPC} ${LIBMSGPACK} ${GLOG_LIBRARIES}
  ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${LIBPTHREAD} ${LIBBOOST_THREAD}
  ${Boost_PROGRAM_OPTIONS_LIBRARY} ${OPENSSL_LIBRARIES}
) 

add_executable(address_test address_test.cc)
add_executable(sync_call sync_call.cc)
add_executable(async_call async_call.cc)
add_executable(callback callback.cc)
add_executable(notify notify.cc)
add_executable(async_server async_server.cc)
add_executable(udp udp.cc)
add_executable(attack_connect attack_connect.cc)
add_executable(attack_pipeline attack_pipeline.cc)
add_executable(attack_huge attack_huge.cc)
add_executable(attack_callback attack_callback.cc)
