cmake_minimum_required(VERSION 3.27.0)

set(PROJECT_NAME FvrRenderer)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_FILE_HEADERS
    fvrrenderer.h
)

set(PROJECT_FILE_SOURCES
    fvrrenderer.cpp
)

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_FILE_HEADERS}
    ${PROJECT_FILE_SOURCES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE FVRRENDERER_LIBRARY)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)
