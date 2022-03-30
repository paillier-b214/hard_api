cmake_minimum_required(VERSION 3.10)
project(hard_api)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(platform)

add_executable(hard_api src/hard_api.cpp include/hard_api.h)
target_link_libraries(hard_api pynq_api pthread cma gmpxx gmp)

