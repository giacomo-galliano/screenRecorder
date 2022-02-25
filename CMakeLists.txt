cmake_minimum_required(VERSION 3.18)
project(screenRecorder LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_subdirectory(lib)
add_subdirectory(test)
