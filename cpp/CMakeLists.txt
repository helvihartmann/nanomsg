#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 2.6)
#Name your project here
project(nanomsg)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS -g)
add_definitions(-std=c++11)

#This tells CMake to do executable

add_executable(pushpullbm src/pushpull_bm.cpp src/parameters.cpp src/functions.cpp src/socketmng.cpp)
target_link_libraries(pushpullbm /usr/local/lib/libnanomsg.a -pthread -lanl)

add_executable(pushpullbmts src/pushpull-bm-ts.cpp src/parameters.cpp src/functions.cpp src/socketmng.cpp)
target_link_libraries(pushpullbmts /usr/local/lib/libnanomsg.a -pthread -lanl)

add_executable(pushpullbmts2 src/pushpull-bm-ts2.cpp src/parameters.cpp src/functions.cpp src/socketmng.cpp)
target_link_libraries(pushpullbmts2 /usr/local/lib/libnanomsg.a -pthread -lanl)

add_executable(reqrepbm src/reqrep_bm.cpp src/parameters.cpp src/functions.cpp src/socketmng.cpp)
target_link_libraries(reqrepbm /usr/local/lib/libnanomsg.a -pthread -lanl)

add_executable(pubsubbm src/pubsub_bm.cpp src/parameters.cpp src/functions.cpp src/socketmng.cpp)
target_link_libraries(pubsubbm /usr/local/lib/libnanomsg.a -pthread -lanl)

add_executable(surveybm src/survey_bm.cpp src/parameters.cpp src/functions.cpp src/socketmng.cpp)
target_link_libraries(surveybm /usr/local/lib/libnanomsg.a -pthread -lanl)