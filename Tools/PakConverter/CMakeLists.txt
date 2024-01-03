cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME PakConverter)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_FILE_HEADERS
    paktools.h
)

set(PROJECT_FILE_SOURCES
    main.cpp
    paktools.cpp
)

add_executable(${PROJECT_NAME}
    ${PROJECT_FILE_HEADERS}
    ${PROJECT_FILE_SOURCES}
)

# LibFVR
target_link_libraries(${PROJECT_NAME} PRIVATE LibFVR)
