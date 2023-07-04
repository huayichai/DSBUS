cmake_minimum_required(VERSION 3.10)
set(BUILD_SHARED_LIBS OFF)              # We expect external libraries to be linked statically.
set(CMAKE_CXX_STANDARD 17)              # Compile as C++17.
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Require C++17 support.

project(ConcurrentTrie VERSION 2023.07)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to `Debug` as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

######################################################################################################################
# DEPENDENCIES
######################################################################################################################

# CTest
enable_testing()

######################################################################################################################
# COMPILER SETUP
######################################################################################################################

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wfatal-errors")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")

set(THIRD_PARTY_INCLUDE_DIR 
    ${PROJECT_SOURCE_DIR}/third_party
    ${PROJECT_SOURCE_DIR}/third_party/fmt/include)

include_directories(${THIRD_PARTY_INCLUDE_DIR})
include_directories(BEFORE src) # This is needed for gtest.

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(third_party)