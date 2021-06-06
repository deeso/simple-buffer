# cmake_minimum_required(VERSION <specify CMake version here>)
project(simple_buffer)
cmake_minimum_required(VERSION 3.19)
include(FindLua)



set(SOURCE "source")
set(INCLUDE "include")
set(TEST "test")
set(_VCPKG_INSTALLED_DIR "/home/apridgen/personal/research/code/vcpkg/installed")
set(_USR_BASE "/usr")

set(CMAKE_CXX_STANDARD 17)

#Prep ourselves for compiling boost
set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost 1.71 COMPONENTS program_options unit_test_framework system filesystem regex REQUIRED)

include_directories (${INCLUDE} ${Boost_INCLUDES} )
link_directories("/usr//lib" ${LIBRARY_OUTPUT_PATH})
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

message (STATUS "This is BINARY dir " ${PROJECT_BINARY_DIR})
message (STATUS "This is SOURCE dir " ${PROJECT_SOURCE_DIR})
message (STATUS "CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS})
message (STATUS "EXECUTABLE_OUTPUT_PATH " ${EXECUTABLE_OUTPUT_PATH})
message (STATUS "LIBRARY_OUTPUT_PATH " ${LIBRARY_OUTPUT_PATH})
message (STATUS "LIBRARY_OUTPUT_PATH " ${Boost_LIBRARIES})

add_library(simple_buffer STATIC
            ${INCLUDE}/library.h
            ${INCLUDE}/allocator.h
            ${INCLUDE}/options.h
            ${SOURCE}/options.cpp
            ${INCLUDE}/options.h
            ${INCLUDE}/buffer.h
            ${SOURCE}/buffer.cpp
            ${INCLUDE}/util.h
            ${SOURCE}/util.cpp
            ${INCLUDE}/clooper.h
        )

add_subdirectory (apps)
