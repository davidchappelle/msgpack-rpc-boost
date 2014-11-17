cmake_minimum_required(VERSION 2.8)

PROJECT(test)

add_executable(address_test address_test.cc asio.cc)
target_link_libraries (address_test mprpc-static)

add_executable(sync_call sync_call.cc asio.cc)
target_link_libraries (sync_call mprpc-static)

add_executable(async_call async_call.cc asio.cc)
target_link_libraries (async_call mprpc-static)

add_executable(callback callback.cc asio.cc)
target_link_libraries (callback mprpc-static)

add_executable(notify notify.cc asio.cc)
target_link_libraries (notify mprpc-static)

add_executable(async_server async_server.cc asio.cc)
target_link_libraries (async_server mprpc-static)

add_executable(udp udp.cc asio.cc)
target_link_libraries (udp mprpc-static)

add_executable(attack_connect attack_connect.cc asio.cc)
target_link_libraries (attack_connect mprpc-static)

add_executable(attack_pipeline attack_pipeline.cc asio.cc)
target_link_libraries (attack_pipeline mprpc-static)

add_executable(attack_huge attack_huge.cc asio.cc)
target_link_libraries (attack_huge mprpc-static)

add_executable(attack_callback attack_callback.cc asio.cc)
target_link_libraries (attack_callback mprpc-static)
