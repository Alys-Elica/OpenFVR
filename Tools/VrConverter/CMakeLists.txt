cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME VrConverter)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_FILE_HEADERS
    fnxvr.h
)

set(PROJECT_FILE_SOURCES
    fnxvr.cpp
    main.cpp
)

add_executable(${PROJECT_NAME}
    ${PROJECT_FILE_HEADERS}
    ${PROJECT_FILE_SOURCES}
)

# LibFVR
target_link_libraries(${PROJECT_NAME} PRIVATE LibFVR)

# Boost
find_package(Boost REQUIRED COMPONENTS program_options)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost Boost::program_options)

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})

