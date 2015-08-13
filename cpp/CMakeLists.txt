#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 2.6)
#Name your project here
project(nanomsg)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS -g)
add_definitions(-std=c++11)

#This tells CMake to do executable

add_executable(pushpullbm pushpull_bm.cpp parameters.cpp)
target_link_libraries(pushpullbm /usr/local/lib/libnanomsg.a -pthread -lanl)

add_executable(pubsubbm pubsub_bm.cpp parameters.cpp)
target_link_libraries(pubsubbm /usr/local/lib/libnanomsg.a -pthread -lanl)