cmake_minimum_required(VERSION 2.8)

project(luci)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

# Uncomment the next line to enable lots of debug output during runtime
#add_definitions(-DNDEBUG -DDEBUG)

#IF GCC
add_definitions(-fno-gcse)
#endif

add_subdirectory(src bin)
