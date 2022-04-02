cmake_minimum_required(VERSION 3.10)
project(OSMP C)

set(CMAKE_C_STANDARD 99)
# Link to needed libraries
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wconversion -Wextra -pthread")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include_directories(.)
#link_libraries(rt)

add_executable(Fork OSMPFork/OSMPFork.c)
add_executable(Echoall OSMPFork/Echoall.c)

add_executable(Demo demo.c)
