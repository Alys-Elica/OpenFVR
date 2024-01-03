cmake_minimum_required(VERSION 3.14.0)

set(PROJECT_NAME LibFVR)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_FILE_HEADERS
    libfvr_globals.h

    fvr_files/fvr_dat.h
    file.h
)

set(PROJECT_FILE_SOURCES
    fvr_files/fvr_dat.cpp
    file.cpp
)

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_FILE_HEADERS}
    ${PROJECT_FILE_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_compile_definitions(
    LIBFVR_EXPORTS
)
