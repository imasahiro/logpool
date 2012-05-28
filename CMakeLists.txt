include(CheckTypeSize)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckLibraryExists)
include(CheckCSourceCompiles)

cmake_minimum_required(VERSION 2.8)
project(logpool)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")

set(PACKAGE_NAME    ${PROJECT_NAME})
set(PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set(PACKAGE_STRING  ${CMAKE_PROJECT_NAME}-${PACKAGE_VERSION})
set(SO_VERSION   ${VERSION_MAJOR}.${VERSION_MINOR})

set(K_CONFIG_ARCH ${CMAKE_SYSTEM})
set(K_CONFIG_OS   ${CMAKE_SYSTEM_NAME})
set(K_CONFIG_CPU  ${CMAKE_SYSTEM_PROCESSOR})

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/lio)
include_directories(${INCLUDE_DIRS})

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" )
endif()

SET(CMAKE_C_FLAGS         "-Wall -Wno-variadic-macros")
SET(CMAKE_CXX_FLAGS       "-Wall -Wno-variadic-macros")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -g")
SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2 -g")
SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -O0 -g3")

set(LOGPOOL_SRC
	logpool.c
	plugins/lpstring.c
#	plugins/file2.c
#	plugins/memcache.c
#	plugins/syslog.c
#	plugins/filter.c
#	plugins/multiplexer.c
	plugins/lio_plugin.c
	map.c
	reactive.c
)

set(LLVM_LOGPOOL_SRC
	jit/llvm.cpp
	jit/bc.cpp
	jit/llvm_file2.cpp
	jit/llvm_memcache.cpp
)

set(LOGPOOL_DAEMON_SRC util/logpoold.c)
set(LOGPOOL_LOGPUT_SRC util/logput.c)
set(LOGPOOL_POOL pool_plugin/pool_plugin.c
        pool_plugin/print.c
        pool_plugin/copy.c
        pool_plugin/statics.c
        pool_plugin/time.c
        pool_plugin/react.c
        pool_plugin/key.c
        pool_plugin/val.c
        pool_plugin/stream.c
        pool_plugin/response.c
        pool_plugin/create.c
)

set(LIO_SRC lio/lio.c lio/stream.c lio/trace.c lio/server.c lio/client.c)

set(LOGPOOL_SRC ${LOGPOOL_SRC} ${LOGPOOL_POOL} ${LIO_SRC})

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


find_library(HAVE_LIBDL        NAMES dl)
find_library(HAVE_LIBM         NAMES m)

check_type_size("void *" SIZEOF_VOIDP)
check_type_size(long     SIZEOF_LONG)
check_type_size(int      SIZEOF_INT)
check_type_size(float    SIZEOF_FLOAT)
check_type_size(double   SIZEOF_DOUBLE)

set(ExtraLibs)

#if(HAVE_LIBMEMCACHED)
#	set(ExtraLibs ${ExtraLibs} ${HAVE_LIBMEMCACHED})
#endif(HAVE_LIBMEMCACHED)
#
#option(USE_LLVM "Use llvm" on)
#if(USE_LLVM)
#set(LOGPOOL_USE_LLVM true)
#endif(USE_LLVM)
#
#if(LOGPOOL_USE_LLVM)
#find_program(LLVM_CONFIG NAMES llvm-config DOC "llvm-config")
#find_program(HAVE_CLANG   NAMES clang DOC "clang")
#if(NOT LLVM_CONFIG)
#set(LOGPOOL_USE_LLVM false)
#endif(NOT LLVM_CONFIG)
#endif(LOGPOOL_USE_LLVM)
#
#if(LLVM_CONFIG)
#execute_process(
#		COMMAND ${LLVM_CONFIG} --version
#		OUTPUT_VARIABLE LLVM_VERSION
#		OUTPUT_STRIP_TRAILING_WHITESPACE
#		)
#
#if(${LLVM_VERSION} VERSION_GREATER "3.0")
#set(LOGPOOL_USE_LLVM_31 1)
#endif(${LLVM_VERSION} VERSION_GREATER "3.0")
#
#set(LOGPOOL_USE_LLVM 1)
#set(LOGPOOL_SRC "${LOGPOOL_SRC};${LLVM_LOGPOOL_SRC}")
#
#execute_process(
#		COMMAND ${LLVM_CONFIG} --cppflags
#		OUTPUT_VARIABLE LLVM_CFLAGS
#		OUTPUT_STRIP_TRAILING_WHITESPACE
#		)
#execute_process(
#		COMMAND ${LLVM_CONFIG} --cxxflags
#		OUTPUT_VARIABLE LLVM_CXXFLAGS
#		OUTPUT_STRIP_TRAILING_WHITESPACE
#		)
#execute_process(
#		COMMAND ${LLVM_CONFIG} --ldflags
#		OUTPUT_VARIABLE LLVM_LDFLAGS
#		OUTPUT_STRIP_TRAILING_WHITESPACE
#		)
#execute_process(
#	COMMAND ${LLVM_CONFIG} --libs core jit native linker ipo engine interpreter
#	OUTPUT_VARIABLE LLVM_JIT_LIBS
#	OUTPUT_STRIP_TRAILING_WHITESPACE
#	)
#execute_process(
#		COMMAND ${LLVM_CONFIG} --libdir
#		OUTPUT_VARIABLE LLVM_LIBDIR
#		OUTPUT_STRIP_TRAILING_WHITESPACE
#		)
#
#set(CMAKE_CXX_FLAGS "${LLVM_CXXFLAGS} ${CMAKE_CXX_FLAGS}")
#string(REPLACE " " ";" v ${LLVM_JIT_LIBS})
#link_directories(${LINK_DIRECTORIES} ${LLVM_LIBDIR})
#set(ExtraLibs ${ExtraLibs} ${v})
#
#add_executable(ConvertBC ./jit/tohex.c)
#if(HAVE_CLANG)
#	string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
#	set(_cflag "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${BUILD_TYPE}}")
#	set(_tmp "${_cflag} -g0")
#	set(_output ${CMAKE_CURRENT_BINARY_DIR}/puts.bc)
#	add_custom_command(
#		OUTPUT  ${_output}
#		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/jit/puts.c
#		COMMAND ${HAVE_CLANG} ${CMAKE_CURRENT_SOURCE_DIR}/jit/puts.c
#		${LLVM_C_FLAGS} -emit-llvm -c -o
#		${_output})
#	add_custom_command(
#		OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/llvm_bc.h
#		DEPENDS ConvertBC ${_output}
#		COMMAND ConvertBC ${_output})
#	set(LOGPOOL_SRC ${LOGPOOL_SRC} ${CMAKE_CURRENT_BINARY_DIR}/llvm_bc.h)
#endif(HAVE_CLANG)
#
#endif(LLVM_CONFIG)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
		${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-fPIC)

set(ExtraLibs ${ExtraLibs} ${HAVE_PTHREAD})
set(ExtraLibs ${ExtraLibs} ${HAVE_LIBEVENT})
set(ExtraLibs ${ExtraLibs} ${HAVE_LIBEVENT_PTHREADS})

add_library(logpool SHARED ${LOGPOOL_SRC})
target_link_libraries(logpool ${ExtraLibs})
set_target_properties(logpool PROPERTIES VERSION ${PACKAGE_VERSION} SOVERSION ${SO_VERSION})

add_definitions(-DLIO_IDL_OUTPUT="${CMAKE_CURRENT_BINARY_DIR}/message.idl.data.h")
add_executable(lio_idlgen lio/idl-gen.cpp)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/message.idl.data.h
        DEPENDS ${PROJECT_SOURCE_DIR}/lio/message.idl
        COMMAND lio_idlgen ${PROJECT_SOURCE_DIR}/lio/message.idl
        )
add_custom_target(CREATE_IDL_H DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/message.idl.data.h)
add_dependencies(logpool CREATE_IDL_H)

# test cases
enable_testing()

add_executable(logpoold ${LOGPOOL_DAEMON_SRC})
target_link_libraries(logpoold logpool ${ExtraLibs})

add_executable(logput ${LOGPOOL_LOGPUT_SRC})
target_link_libraries(logput logpool ${ExtraLibs})

add_executable(logtail util/logtail.c)
target_link_libraries(logtail logpool ${ExtraLibs})

macro(add_clib_test test_name)
	add_executable(${test_name} ${ARGN})
	target_link_libraries(${test_name} ${TestLibs})
	add_test(${test_name} ${test_name})
endmacro(add_clib_test test_name)

set(TestLibs logpool)
if(HAVE_LIBDL)
	set(TestLibs ${TestLibs} ${HAVE_LIBDL})
endif(HAVE_LIBDL)

#add_clib_test(test_file     test/file.c)
#add_clib_test(test_memcache test/memcache.c)
add_clib_test(test_string   test/string.c)
#add_clib_test(test_syslog   test/syslog.c)
#add_clib_test(test_cpp      test/cpp.cpp)
#add_clib_test(test_largefmt test/large_fmtdata.c)
#add_clib_test(test_filter   test/filter.c)
#add_clib_test(test_multi    test/multiplex.c)
#add_clib_test(test_event    test/event.c)
add_clib_test(test_trace   test/trace.c)
add_clib_test(test_trace2  test/trace2.c)
add_executable(test_map    test/map.c)
target_link_libraries(test_map ${TestLibs})
add_test(test_map test_map ${CMAKE_CURRENT_SOURCE_DIR}/test/map-test.txt)

add_clib_test(test_react test/react.c)
#add_clib_test(test_list  test/list.c)
add_clib_test(test_pool  test/pool.c ${LOGPOOL_POOL})

if(LOGPOOL_USE_LLVM)
add_clib_test(test_llvm    test/llvm.c)
endif(LOGPOOL_USE_LLVM)

install(TARGETS logpool
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/logpool.h DESTINATION include)

