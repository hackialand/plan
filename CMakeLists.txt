cmake_minimum_required(VERSION 4.0)
project(plan)

set(CMAKE_CXX_STANDARD 26)
add_library(Plan STATIC Plan.cpp)
add_executable(plan main.cpp)

install(TARGETS Plan DESTINATION lib)
install(FILES Plan.h DESTINATION include)
install(TARGETS plan DESTINATION bin)
