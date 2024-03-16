cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME LstParser)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME} SHARED
    liblstparser_globals.h

    lstparser.h
    lstparser.cpp

    lstscript.h
    lstscript.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBLSTPARSER_EXPORTS)
