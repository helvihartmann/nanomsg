#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 2.6)
#Name your project here
project(nanomsg)

set(CMAKE_CXX_COMPILER gcc)
set(CMAKE_CXX_FLAGS -g)
add_definitions(-std=c99)

#This tells CMake to do executable
add_executable(pipeline pipeline.c)
target_link_libraries(pipeline /usr/local/lib/libnanomsg.a -pthread -lanl)

add_executable(pingpong pingpong.c)
target_link_libraries(pingpong /usr/local/lib/libnanomsg.a -pthread -lanl)

add_executable(pingpongint pingpong_int.c)
target_link_libraries(pingpongint /usr/local/lib/libnanomsg.a -pthread -lanl)
