cmake_minimum_required(VERSION 3.12)
project(avconnect VERSION 0.1 LANGUAGES C CXX)

include(cmake/xplm.cmake)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
	set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version" FORCE)
endif()

option(LIBACFUTILS "Directory where libacfutils is installed" "libacfutils")

find_package(OpenGL REQUIRED)
use_static_libc()
find_acfutils(${LIBACFUTILS})
find_xplane_sdk("${LIBACFUTILS}/SDK")


add_subdirectory(lib)
add_subdirectory(src)
