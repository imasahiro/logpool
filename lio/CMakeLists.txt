include(CheckTypeSize)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckLibraryExists)
include(CheckCSourceCompiles)

cmake_minimum_required(VERSION 2.6)
project(lio)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

SET(CMAKE_C_FLAGS         "-Wall")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -g")
SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")

set(LIO_SRC lio.c stream.c trace.c server.c client.c query.c qengine.c)
find_library(HAVE_LIBDL    NAMES dl)
find_library(HAVE_PTHREAD  NAMES pthread)
find_library(HAVE_LIBEVENT NAMES event)
find_library(HAVE_LIBEVENT_PTHREADS NAMES event_pthreads)
check_include_files(pthread.h      HAVE_PTHREAD_H)
check_include_files(event2/event.h HAVE_LIBEVENT_H)

if(NOT HAVE_PTHREAD_H)
	message(FATAL_ERROR "pthread.h not found: ${HAVE_PTHREAD_H}")
endif(NOT HAVE_PTHREAD_H)

if(NOT HAVE_LIBEVENT_H)
	message(FATAL_ERROR "event2/event.h not found")
endif(NOT HAVE_LIBEVENT_H)

if(NOT HAVE_PTHREAD)
	message(FATAL_ERROR "libpthread not found")
endif(NOT HAVE_PTHREAD)

if(NOT HAVE_LIBEVENT)
	message(FATAL_ERROR "libevent not found")
endif(NOT HAVE_LIBEVENT)

check_type_size("void *" SIZEOF_VOIDP)
check_type_size(long     SIZEOF_LONG)
check_type_size(int      SIZEOF_INT)
check_type_size(float    SIZEOF_FLOAT)
check_type_size(double   SIZEOF_DOUBLE)

set(ExtraLibs)

set(ExtraLibs ${ExtraLibs} ${HAVE_PTHREAD})
set(ExtraLibs ${ExtraLibs} ${HAVE_LIBEVENT})
set(ExtraLibs ${ExtraLibs} ${HAVE_LIBEVENT_PTHREADS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set(LIO_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
#		${CMAKE_CURRENT_BINARY_DIR}/config.h)
#add_definitions(-DHAVE_CONFIG_H)

# test cases
enable_testing()

add_library(lio ${LIO_SRC})
target_link_libraries(lio ${ExtraLibs})

add_definitions(-DLIO_IDL_OUTPUT="${CMAKE_CURRENT_BINARY_DIR}/message.idl.data.h")
add_executable(lio_idlgen idl-gen.cpp)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/message.idl.data.h
        DEPENDS ${PROJECT_SOURCE_DIR}/message.idl
        COMMAND lio_idlgen ${PROJECT_SOURCE_DIR}/message.idl
        )
add_custom_target(CREATE_IDL_H DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/message.idl.data.h)
add_dependencies(lio CREATE_IDL_H)

macro(add_c_test _NAME)
	add_executable(${_NAME} ${ARGN})
	target_link_libraries(${_NAME} lio)
endmacro(add_c_test SCR)

add_c_test(test_t test/test_t.c)
add_c_test(test_s test/test_s.c)
add_c_test(test_c test/test_c.c)
add_c_test(test_q test/test_q.c)
add_c_test(test_e test/test_e.c)
