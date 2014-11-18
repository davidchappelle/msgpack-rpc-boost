set(MSGPACK_RPC_LIBRARY mprpc)

add_executable(address_test address_test.cc asio.cc)
add_dependencies(address_test ${MSGPACK_RPC_LIBRARY})
target_link_libraries (address_test ${MSGPACK_RPC_LIBRARY})

add_executable(sync_call sync_call.cc asio.cc)
add_dependencies(sync_call ${MSGPACK_RPC_LIBRARY})
target_link_libraries (sync_call ${MSGPACK_RPC_LIBRARY})

add_executable(async_call async_call.cc asio.cc)
add_dependencies(async_call ${MSGPACK_RPC_LIBRARY})
target_link_libraries (async_call ${MSGPACK_RPC_LIBRARY})

add_executable(callback callback.cc asio.cc)
add_dependencies(callback ${MSGPACK_RPC_LIBRARY})
target_link_libraries (callback ${MSGPACK_RPC_LIBRARY})

add_executable(notify notify.cc asio.cc)
add_dependencies(notify ${MSGPACK_RPC_LIBRARY})
target_link_libraries (notify ${MSGPACK_RPC_LIBRARY})

add_executable(async_server async_server.cc asio.cc)
add_dependencies(async_server ${MSGPACK_RPC_LIBRARY})
target_link_libraries (async_server ${MSGPACK_RPC_LIBRARY})

add_executable(udp udp.cc asio.cc)
add_dependencies(udp ${MSGPACK_RPC_LIBRARY})
target_link_libraries (udp ${MSGPACK_RPC_LIBRARY})

add_executable(attack_connect attack_connect.cc asio.cc)
add_dependencies(attack_connect ${MSGPACK_RPC_LIBRARY})
target_link_libraries (attack_connect ${MSGPACK_RPC_LIBRARY})

add_executable(attack_pipeline attack_pipeline.cc asio.cc)
add_dependencies(attack_pipeline ${MSGPACK_RPC_LIBRARY})
target_link_libraries (attack_pipeline ${MSGPACK_RPC_LIBRARY})

add_executable(attack_huge attack_huge.cc asio.cc)
add_dependencies(attack_huge ${MSGPACK_RPC_LIBRARY})
target_link_libraries (attack_huge ${MSGPACK_RPC_LIBRARY})

add_executable(attack_callback attack_callback.cc asio.cc)
add_dependencies(attack_callback ${MSGPACK_RPC_LIBRARY})
target_link_libraries (attack_callback ${MSGPACK_RPC_LIBRARY})
