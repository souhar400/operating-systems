cmake_minimum_required(VERSION 3.10)
project(OSMP C)

set(CMAKE_C_STANDARD 99)
# Link to needed libraries
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wconversion -Wextra -pthread")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include_directories(.)
link_libraries(pthread)
link_libraries(rt)

add_executable(Echoall OSMPFork/Echoall.c)
add_executable(Demo demo.c lib/OSMPlib.h)
add_executable(OSMPRun OSMPRun/OSMPRun.c lib/OSMPlib.h lib/OSMPlib.c)
add_executable(OSMPExecutable OSMPRun/OSMPExecutable.c lib/OSMPlib.h lib/OSMPlib.c)
add_executable(OSMPBcast OSMPRun/OSMPBcast.c lib/OSMPlib.h lib/OSMPlib.c)
add_executable(OSMPNonBlocking OSMPRun/OSMPNonBlocking.c lib/OSMPlib.h lib/OSMPlib.c )

#target_link_libraries(OSMPRun -lrt)
#target_link_libraries(OSMPExecutable -lrt)

#add_executable(Fork OSMPFork/OSMPFork.c)
#add_executable(SimpleFork experimente/simplesFork/fork.c)
#add_executable(ZweiFork experimente/zweimalfork/zweifork.c)
#add_executable(Aufgabe2 experimente/Aufgabe2/aufgabe2.c)
#add_executable(Aufgabe3 experimente/Aufgabe3/aufgabe3.c)
#add_executable(LokGlobMem experimente/lok_glob_mem/lokglobmem.c)
#add_executable(Execl_exp experimente/exec/execl_exp.c)
#add_executable(SchleifeFork experimente/zweimalfork/fork_in_schleife.c)
