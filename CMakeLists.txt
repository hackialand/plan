cmake_minimum_required(VERSION 4.0)
project(plan)

set(CMAKE_CXX_STANDARD 26)

add_executable(plan main.cpp)

install(TARGETS plan DESTINATION bin)
