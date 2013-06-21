project(mprpc)
cmake_minimum_required(VERSION 2.6)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(BUILD_TESTS $ENV{BUILD_TESTS})

##########################################
add_definitions(-DGOOGLE_STRIP_LOG=1)
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-g -O -Wall")
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

INCLUDE(FindBoost)
IF(Boost_FOUND)
	SET(Boost_USE_MULTITHREADED ON)
	SET(Boost_ADDITIONAL_VERSIONS "1.46.1" "1.40.0")
	FIND_PACKAGE(Boost COMPONENTS system thread filesystem regex)
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
	LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ELSE(Boost_FOUND)
	MESSAGE(FATAL_ERROR "The boost library was not found on your system.")
ENDIF(Boost_FOUND)

find_package(OpenSSL QUIET)
if (NOT OPENSSL_FOUND)
  set(LIBCRYPTO "crypto")
  set(LIBSSL "ssl")
  set(OPENSSL_LIBRARIES ${LIBCRYPTO} ${LIBSSL})
endif(NOT OPENSSL_FOUND)
include_directories(${OPENSSL_INCLUDE_DIR})
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
