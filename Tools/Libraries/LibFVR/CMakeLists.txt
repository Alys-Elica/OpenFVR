cmake_minimum_required(VERSION 3.14.0)
include(CheckIncludeFiles)
include(CheckFunctionExists)

set(PROJECT_NAME LibFVR)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_FILE_HEADERS
    libfvr_globals.h

    fvr/datastream.h
    fvr/image.h

    fvr_files/fvr_4xm.h
    fvr_files/fvr_arnvit.h
    fvr_files/fvr_script.h
)
    
set(PROJECT_FILE_SOURCES
    fvr/datastream.cpp
    fvr/image.cpp
    
    fvr_files/fvr_4xm.cpp
    fvr_files/fvr_arnvit.cpp
    fvr_files/fvr_script.cpp
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

# LibPNG
find_package(PNG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
