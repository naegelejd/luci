cmake_minimum_required(VERSION 2.8)

project(luci)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG")
# Uncomment the next line to enable lots of debug output during runtime
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")

add_subdirectory(src bin)
