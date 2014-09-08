project(mprpc)
cmake_minimum_required(VERSION 2.8)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(BUILD_TESTS $ENV{BUILD_TESTS})

##########################################
add_definitions(-DGOOGLE_STRIP_LOG=1)
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-std=c++11 -g -O -Wall")
endif(CMAKE_COMPILER_IS_GNUCXX)

INCLUDE_DIRECTORIES(./src ../include)

if (MSVC)
  # Windows uses Unicode internally, so we also use Unicode
  add_definitions(-DUNICODE -D_UNICODE)
  # Require at least Windows XP.
  add_definitions(-D_WIN32_WINNT=0x0501)
  # autolink for boost::asio does wrongfully require date_time and regex libraries.
  # See: http://www.boost.org/doc/libs/1_46_1/doc/html/boost_asio/using.html
  add_definitions(-DBOOST_DATE_TIME_NO_LIB -DBOOST_REGEX_NO_LIB)
endif(MSVC)

if (NOT WIN32)
  SET(LIBPTHREAD "pthread")
endif(NOT WIN32)
IF(APPLE)
  SET(Boost_USE_STATIC_LIBS true)
ENDIF(APPLE)
IF(WIN32)
  SET(Boost_USE_STATIC_LIBS true)
ENDIF(WIN32)

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

#Macro for the caller.hmpl & cclog.hmpl (any more ?)
MACRO(mplex SOURCE)
  set(OUTFILE ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}.h)
  set(INFILE ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}.hmpl)

  add_custom_command(OUTPUT ${OUTFILE}
    DEPENDS ${INFILE}
    COMMAND ruby -I${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/mplex -rmpl ${INFILE} -o ${OUTFILE}
    COMMENT "processing ${SOURCE}.hmpl file..")
ENDMACRO(mplex)

ADD_SUBDIRECTORY(src/msgpack/rpc)

if(BUILD_TESTS)
MESSAGE("-- Build test applications")
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(unittest)
endif(BUILD_TESTS)
