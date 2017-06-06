cmake_minimum_required(VERSION 3.3)
project(PRy)

set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_PREFIX_PATH "/home/agentomek/.openmpi")

find_package(MPI REQUIRED)
find_package(Threads)
include_directories(SYSTEM ${MPI_CXX_INCLUDE_PATH})

set(SOURCE_FILES main.cpp)

add_executable(PRy ${SOURCE_FILES})

target_link_libraries(PRy ${MPI_CXX_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})