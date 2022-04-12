cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME
    PynqLibrary
)
set(LIBRARY_NAME
    pynq
)
set(LIBRARY_HEADERS_DIR
    include/${LIBRARY_NAME}
)
set(LIBRARY_HEADERS
    ${LIBRARY_HEADERS_DIR}/pynq_api.h
    ${LIBRARY_HEADERS_DIR}/drivers.h
)
set(LIBRARY_SOURCE_DIR
    src
)
set(LIBRARY_SOURCE
    ${LIBRARY_SOURCE_DIR}/pynq_api.c
)

project(${PROJECT_NAME})

add_library(${LIBRARY_NAME} STATIC
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCE}
)

target_include_directories(${LIBRARY_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${LIBRARY_NAME}>
    $<INSTALL_INTERFACE:include/${LIBRARY_NAME}>
)

target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
