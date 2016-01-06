cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(shm-example)

set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -ggdb -msse4.2 -Wall -pedantic -Wextra -Winit-self -Wundef -Wold-style-cast -Woverloaded-virtual -Wwrite-strings -Wnon-virtual-dtor -fno-omit-frame-pointer")

find_package(Boost 1.54.0 REQUIRED)
find_package(Threads)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_definitions(-DBOOST_ALL_DYN_LINK)

link_libraries(pthread rt /usr/local/lib/libnanomsg.a -lanl -pthread)
add_executable(server src/server.cpp src/socketmng.cpp src/parameters.cpp)
add_executable(agent src/agent.cpp src/socketmng.cpp src/parameters.cpp)
add_executable(shm-writer src/shm-writer.cpp src/shmmanager.cpp src/socketmng.cpp src/parameters.cpp)
add_executable(shm-reader src/shm-reader.cpp src/shmmanager.cpp src/socketmng.cpp src/parameters.cpp)
