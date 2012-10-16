cmake_minimum_required(VERSION 2.8)

project(luci)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g -DDEBUG")

add_subdirectory(src bin)
