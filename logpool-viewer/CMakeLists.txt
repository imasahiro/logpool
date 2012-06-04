cmake_minimum_required(VERSION 2.6)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

SET(CMAKE_C_FLAGS         "-Wall")
SET(CMAKE_CXX_FLAGS       "-Wall")
SET(CMAKE_C_FLAGS_RELEASE      "-Wall -O2 -g")
SET(CMAKE_C_FLAGS_DEBUG        "-O0 -g3")
SET(CMAKE_C_FLAGS_COMPRESSED   "-Os -g0")
SET(CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_C_FLAGS_RELEASE}")
SET(CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_C_FLAGS_DEBUG}")
SET(CMAKE_CXX_FLAGS_COMPRESSED "${CMAKE_C_FLAGS_COMPRESSED}")

set(VIEWER_SRC
	main.cpp
	viewer.cpp
	ellipse.cpp
	fix_grouping_bug.cpp
	info.cpp
	node.cpp
	rect.cpp
	scene.cpp
	text.cpp
)
set(PACKAGE_HEADERS viewer.hpp util.hpp)

project(logpoolv)
set(QT_USE_QTWEBKIT TRUE)
find_package(Qt4)
if(QT4_FOUND)
message(status "HI")
	qt4_wrap_cpp(PACKAGE_HEADERS_MOC ${PACKAGE_HEADERS})
	include(${QT_USE_FILE})
	include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
	add_definitions(${QT_DEFINITIONS})
	add_executable(logpoolv ${VIEWER_SRC} ${PACKAGE_HEADERS_MOC})
	target_link_libraries(logpoolv
		${QT_QTCORE_LIBRARY}
		${QT_QTWEBKIT_LIBRARY}
		${QT_QTGUI_LIBRARY}
		${QT_QTNETWORK_LIBRARY}
		${QT_QTOPENGL_LIBRARY}
	)
	install(TARGETS logpoolv RUNTIME DESTINATION bin)
endif(QT4_FOUND)
