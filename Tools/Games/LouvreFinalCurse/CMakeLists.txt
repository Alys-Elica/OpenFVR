cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME LouvreFinalCurse)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_FILE_HEADERS
    pluginlouvre.h
)

set(PROJECT_FILE_SOURCES
    pluginlouvre.cpp

    main.cpp
)

add_executable(${PROJECT_NAME}
    ${PROJECT_FILE_HEADERS}
    ${PROJECT_FILE_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE FvrEngine)
target_link_libraries(${PROJECT_NAME} PRIVATE LibFVR)

# Converter
add_executable(LouvreConverter
    converter.cpp
)
target_link_libraries(LouvreConverter PRIVATE LibFVR)
