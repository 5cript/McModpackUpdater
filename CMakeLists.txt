# Version check
cmake_minimum_required (VERSION 3.8)

if(NOT "${CMAKE_CXX_STANDARD}")
	set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
  endif()
endif()

set(proj "mod_updater")

# Project
project(${proj})

# Add files
file(GLOB sources "*.cpp")

# Add library
add_executable(${proj} ${sources})

target_include_directories(${proj} PRIVATE "..")

find_library(LSIMPLEJSON SimpleJSON PATHS "../SimpleJSON/build")
find_library(LATTENDEE attendee PATHS "../attendee/build")
find_library(LCURL curl)

target_link_libraries(
	${proj}
	${LSIMPLEJSON}
	${LATTENDEE}
	${LCURL}
	-lstdc++fs
)

# Compiler Options
set(DEBUG_OPTIONS -fexceptions -g -Wall -pedantic-errors -pedantic)
set(RELEASE_OPTIONS -fexceptions -O3 -Wall -pedantic-errors -pedantic -static -static-libstdc++ -static-libgcc)
target_compile_options(${proj} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")
target_compile_options(${proj} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")