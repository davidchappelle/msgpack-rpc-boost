project(mprpc)
cmake_minimum_required(VERSION 2.8)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(BUILD_TESTS $ENV{BUILD_TESTS})

##########################################
add_definitions(-DGOOGLE_STRIP_LOG=1)
set (CMAKE_CXX_FLAGS "-std=c++11 -Wall ${CMAKE_CXX_FLAGS}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src ../include)

find_package (Boost REQUIRED COMPONENTS COMPONENTS system thread filesystem regex)
find_package (OpenSSL QUIET)
find_library (MSGPACK_LIBRARY NAMES "msgpack")
if (MSGPACK_LIBRARY-NOTFOUND)
    message (FATAL_ERROR "The msgpack library was not found on your system.")
endif (MSGPACK_LIBRARY-NOTFOUND)
find_path (MSGPACK_INCLUDE_DIR NAMES "msgpack.hpp" )
if (MSGPACK_INCLUDE_DIR-NOTFOUND)
    message (FATAL_ERROR "The msgpack header was not found on your system.")
endif (MSGPACK_INCLUDE_DIR-NOTFOUND)

find_library (GLOG_LIBRARY NAMES "glog")
if (GLOG_LIBRARY-NOTFOUND)
    message (FATAL_ERROR "The glog library was not found on your system.")
endif (GLOG_LIBRARY-NOTFOUND)
find_path (GLOG_INCLUDE_DIR NAMES "logging.h" PATH_SUFFIXES glog)
if (GLOG_INCLUDE_DIR-NOTFOUND)
    message (FATAL_ERROR "The glog header was not found on your system.")
endif (GLOG_INCLUDE_DIR-NOTFOUND)

include_directories (${Boost_INCLUDE_DIRS}
  ${MSGPACK_INCLUDE_DIR}
  ${GLOG_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIRS})

link_libraries (${Boost_LIBRARIES}
    ${MSGPACK_LIBRARY}
    ${GLOG_LIBRARY}
    ${OPENSSL_LIBRARIES})

##########################################

ADD_SUBDIRECTORY(src/msgpack/rpc)

if(BUILD_TESTS)
MESSAGE("-- Build test applications")
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(unittest)
endif(BUILD_TESTS)
