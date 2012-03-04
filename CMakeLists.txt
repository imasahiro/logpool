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

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${INCLUDE_DIRS})

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" )
endif()

SET(CMAKE_C_FLAGS         "-Wall -Wno-variadic-macros -pedantic")
SET(CMAKE_CXX_FLAGS       "-Wall -Wno-variadic-macros -pedantic")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -g")
SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2 -g")
SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -O0 -g3")

set(LOGPOOL_SRC
	logpool.c
	plugins/lpstring.c
	plugins/file2.c
	plugins/memcache.c
	plugins/syslog.c
	plugins/filter.c
)
set(LLVM_LOGPOOL_SRC
	jit/llvm.cpp
	jit/bc.cpp
	jit/llvm_file2.cpp
	jit/llvm_memcache.cpp
)

find_library(HAVE_LIBDL        NAMES dl)
find_library(HAVE_LIBM         NAMES m)
find_library(HAVE_LIBMEMCACHED NAMES memcached)

check_type_size("void *" SIZEOF_VOIDP)
check_type_size(long     SIZEOF_LONG)
check_type_size(int      SIZEOF_INT)
check_type_size(float    SIZEOF_FLOAT)
check_type_size(double   SIZEOF_DOUBLE)

set(ExtraLibs)
set(MessageLibs)

if(HAVE_LIBMEMCACHED)
	set(ExtraLibs ${ExtraLibs} ${HAVE_LIBMEMCACHED})
	set(MessageLibs ${MessageLibs} memcached)
endif(HAVE_LIBMEMCACHED)

option(USE_LLVM "Use llvm" on)
if(USE_LLVM)
set(LOGPOOL_USE_LLVM true)
endif(USE_LLVM)

if(LOGPOOL_USE_LLVM)
find_program(LLVM_CONFIG NAMES llvm-config DOC "llvm-config")
find_program(HAVE_CLANG   NAMES clang DOC "clang")
if(NOT LLVM_CONFIG)
set(LOGPOOL_USE_LLVM false)
endif(NOT LLVM_CONFIG)
endif(LOGPOOL_USE_LLVM)

if(LLVM_CONFIG)
execute_process(
		COMMAND ${LLVM_CONFIG} --version
		OUTPUT_VARIABLE LLVM_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

if(${LLVM_VERSION} VERSION_GREATER "3.0")
set(LOGPOOL_USE_LLVM_31 1)
endif(${LLVM_VERSION} VERSION_GREATER "3.0")

set(LOGPOOL_USE_LLVM 1)
set(LOGPOOL_SRC "${LOGPOOL_SRC};${LLVM_LOGPOOL_SRC}")

execute_process(
		COMMAND ${LLVM_CONFIG} --cppflags
		OUTPUT_VARIABLE LLVM_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
execute_process(
		COMMAND ${LLVM_CONFIG} --cxxflags
		OUTPUT_VARIABLE LLVM_CXXFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
execute_process(
		COMMAND ${LLVM_CONFIG} --ldflags
		OUTPUT_VARIABLE LLVM_LDFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
execute_process(
	COMMAND ${LLVM_CONFIG} --libs core jit native linker ipo engine interpreter
	OUTPUT_VARIABLE LLVM_JIT_LIBS
	OUTPUT_STRIP_TRAILING_WHITESPACE
	)
execute_process(
		COMMAND ${LLVM_CONFIG} --libdir
		OUTPUT_VARIABLE LLVM_LIBDIR
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

set(CMAKE_CXX_FLAGS "${LLVM_CXXFLAGS} ${CMAKE_CXX_FLAGS}")
string(REPLACE " " ";" v ${LLVM_JIT_LIBS})
link_directories(${LINK_DIRECTORIES} ${LLVM_LIBDIR})
set(ExtraLibs ${ExtraLibs} ${v})

add_executable(ConvertBC ./jit/tohex.c)
if(HAVE_CLANG)
	string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
	set(_cflag "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${BUILD_TYPE}}")
	set(_tmp "${_cflag} -g0")
	set(_output ${CMAKE_CURRENT_BINARY_DIR}/puts.bc)
	add_custom_command(
		OUTPUT  ${_output}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/jit/puts.c
		COMMAND ${HAVE_CLANG} ${CMAKE_CURRENT_SOURCE_DIR}/jit/puts.c
		${LLVM_C_FLAGS} -emit-llvm -c -o
		${_output})
	add_custom_command(
		OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/llvm_bc.h
		DEPENDS ConvertBC ${_output}
		COMMAND ConvertBC ${_output})
	set(LOGPOOL_SRC ${LOGPOOL_SRC} ${CMAKE_CURRENT_BINARY_DIR}/llvm_bc.h)
endif(HAVE_CLANG)

endif(LLVM_CONFIG)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
		${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_definitions(-DHAVE_CONFIG_H)

add_library(logpool SHARED ${LOGPOOL_SRC})
target_link_libraries(logpool ${ExtraLibs})
set_target_properties(logpool PROPERTIES VERSION ${PACKAGE_VERSION} SOVERSION ${SO_VERSION})

add_executable(test_file     test/file.c)
add_executable(test_memcache test/memcache.c)
add_executable(test_string   test/string.c)
add_executable(test_syslog   test/syslog.c)
add_executable(test_cpp      test/cpp.cpp)
add_executable(test_largefmt test/large_fmtdata.c)
add_executable(test_filter   test/filter.c)

set(TestLibs logpool)
if(HAVE_LIBDL)
set(TestLibs ${TestLibs} ${HAVE_LIBDL})
endif(HAVE_LIBDL)
target_link_libraries(test_file     ${TestLibs})
target_link_libraries(test_memcache ${TestLibs})
target_link_libraries(test_string   ${TestLibs})
target_link_libraries(test_syslog   ${TestLibs})
target_link_libraries(test_cpp      ${TestLibs})
target_link_libraries(test_largefmt ${TestLibs})
target_link_libraries(test_filter   ${TestLibs})

# test cases
enable_testing()
add_test(test_string   test_string)
add_test(test_file     test_file)
add_test(test_memcache test_memcache)
add_test(test_syslog   test_syslog)
add_test(test_cpp      test_cpp)
add_test(test_largefmt test_largefmt)
add_test(test_filter   test_filter)

if(LOGPOOL_USE_LLVM)
add_executable(test_llvm test/llvm.c)
target_link_libraries(test_llvm ${TestLibs})
add_test(test_llvm test_llvm)
endif(LOGPOOL_USE_LLVM)


install(TARGETS logpool
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/logpool.h DESTINATION include)

