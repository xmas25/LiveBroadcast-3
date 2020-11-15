# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("LiveBroadcastServer")

set(CMAKE_CXX_FLAGS "-g -Wall -Werror")

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory("network")
add_subdirectory("utils")
add_subdirectory("server")

add_executable(main "main.cpp")
target_link_libraries(main network utils)