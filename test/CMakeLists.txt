cmake_minimum_required(VERSION 2.8)

PROJECT(test)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")

link_libraries(mrpc-static)

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
