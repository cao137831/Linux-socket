cmake_minimum_required(VERSION 3.0)
project(test)

set(HOME /home/cao/Desktop/Linux/socket_cp/)
set(EXECUTABLE_OUTPUT_PATH ${HOME}/bin)


file(GLOB_RECURSE server 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pool/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/socket/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp)
file(GLOB_RECURSE client 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pool/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/socket/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp)

add_executable(server ${server})
add_executable(client ${client})