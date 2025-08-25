cmake_minimum_required(VERSION 4.0)
project(plan)

set(CMAKE_CXX_STANDARD 26)
add_library(Plan STATIC Plan.cpp PlanManager.cpp ExecutionEngine.cpp ExecutionEngine.h Plan.h Layer.cpp PlanManager.h Layer.h)
add_executable(plan main.cpp)

install(TARGETS Plan DESTINATION lib)
install(FILES Plan.h DESTINATION include)
install(TARGETS plan DESTINATION bin)
