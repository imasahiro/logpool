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

SET(CMAKE_C_FLAGS         "-Wall -Wno-variadic-macros")
SET(CMAKE_CXX_FLAGS       "-Wall -Wno-variadic-macros")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -g")
SET(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2 -g")
SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -O0 -g3")

set(LOGPOOL_SRC
	logpool.c
	lpstring.c
	plugins/file2.c
	plugins/memcache.c
	plugins/syslog.c
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
set(LOGPOOL_USE_LLVM 1)
set(LOGPOOL_SRC "${LOGPOOL_SRC};${LLVM_LOGPOOL_SRC}")
find_program(LLVM_CONFIG NAMES llvm-config DOC "llvm-config")
execute_process(
		COMMAND ${LLVM_CONFIG} --version
		OUTPUT_VARIABLE LLVM_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

if(${LLVM_VERSION} VERSION_GREATER "3.0")
set(LOGPOOL_USE_LLVM_31 1)
endif()

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
set(CMAKE_C_FLAGS "${LLVM_CFLAGS} ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${LLVM_CXXFLAGS} ${CMAKE_CXX_FLAGS}")
set(KONOHA_SRC ${KONOHA_SRC} src/lang/llvm.cpp)
string(REPLACE " " ";" v ${LLVM_JIT_LIBS})
link_directories(${LINK_DIRECTORIES} ${LLVM_LIBDIR})
set(ExtraLibs ${ExtraLibs} ${v})
endif(USE_LLVM)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
		${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_definitions(-DHAVE_CONFIG_H)

add_library(logpool SHARED ${LOGPOOL_SRC})
target_link_libraries(logpool ${ExtraLibs})
set_target_properties(logpool PROPERTIES VERSION ${PACKAGE_VERSION} SOVERSION ${SO_VERSION})

install(TARGETS logpool
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/logpool.h DESTINATION include)

MESSAGE(STATUS)
MESSAGE(STATUS "---------------------------------------------------------------------------" )
MESSAGE(STATUS "CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
MESSAGE(STATUS "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
MESSAGE(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "Change a value with: cmake -D<Variable>=<Value>" )
MESSAGE(STATUS "---------------------------------------------------------------------------" )
MESSAGE(STATUS)

