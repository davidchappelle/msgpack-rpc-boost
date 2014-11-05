project(mprpc)

IF(NOT DEFINED VERSION_MAJOR)
  SET(VERSION_MAJOR 0)
ENDIF(NOT DEFINED VERSION_MAJOR)
IF(NOT DEFINED VERSION_MINOR)
  SET(VERSION_MAJOR 0)
ENDIF(NOT DEFINED VERSION_MINOR)
SET(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")

CONFIGURE_FILE(version.h.in version.h)

#if (NOT WIN32)
#mplex(caller)
#endif(NOT WIN32)

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 -g -O -Wall")
endif(CMAKE_COMPILER_IS_GNUCXX)

INCLUDE_DIRECTORIES(../..)

SET(MSGPACK_RPC_SRC
	caller.h
	address.cc
	buffer.cc
	client.cc
	exception.cc
	future.cc
	loop.cc
	reqtable.cc
	request.cc
	server.cc
	session.cc
	session_pool.cc
)

SET(MSGPACK_RPC_TRANSPORT_SRC
    transport/tcp.cc
    transport/stream_handler.cc
    transport/udp.cc
    transport/dgram_handler.cc
)

ADD_LIBRARY(mprpc-static STATIC
    ${MSGPACK_RPC_SRC}
    ${MSGPACK_RPC_TRANSPORT_SRC}
)

ADD_LIBRARY(mprpc-shared SHARED
    ${MSGPACK_RPC_SRC}
    ${MSGPACK_RPC_TRANSPORT_SRC}
)

SET_TARGET_PROPERTIES(mprpc-static PROPERTIES OUTPUT_NAME mprpc)
SET_TARGET_PROPERTIES(mprpc-shared PROPERTIES OUTPUT_NAME mprpc)

#-----------------------------------------------------------------------------
# To fix compilation problem: relocation R_X86_64_32 against `a local symbol' can not be
# used when making a shared object; recompile with -fPIC
# See http://www.cmake.org/pipermail/cmake/2007-May/014350.html
#
IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  SET_TARGET_PROPERTIES(mprpc-shared PROPERTIES COMPILE_FLAGS "-fPIC")
ENDIF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")

##########################################

SET(MPRPC_HEADERS
	atomic_ops.h
	address.h
	buffer.h
	caller.h
	client.h
	exception.h
	future.h
	impl_fwd.h
	loop.h
	protocol.h
	request.h
	server.h
	session.h
	session_pool.h
	types.h
	transport.h
)

SET(MPRPC_TRANSPORT_HEADERS
	transport/tcp.h
	transport/udp.h
)

INSTALL(FILES ${MPRPC_HEADERS} DESTINATION include/msgpack/rpc)
INSTALL(FILES ${MPRPC_TRANSPORT_HEADERS} DESTINATION include/msgpack/rpc/transport)
INSTALL(TARGETS mprpc-static mprpc-shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
