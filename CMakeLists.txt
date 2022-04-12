cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)


set(PLATFORM_DIR platform)
set(PLATFORM_LIBRARIES pynq)

project(Paillier)

add_library(paillier_api STATIC src/base_paillier.cpp src/pynq.cpp)

target_compile_definitions(paillier_api PUBLIC PYNQ)

target_include_directories(paillier_api PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/paillier
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/external
)
target_include_directories(paillier_api PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

foreach(SUB_LIB ${PLATFORM_LIBRARIES})
    add_subdirectory("${PLATFORM_DIR}/${SUB_LIB}")
endforeach()

target_link_libraries(paillier_api ${PLATFORM_LIBRARIES} pthread cma gmpxx gmp)

add_subdirectory(tests)
