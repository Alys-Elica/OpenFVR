cmake_minimum_required(VERSION 3.14.0)

set(PROJECT_NAME MtrConverter)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
    main.cpp
)

# LibFVR
target_link_libraries(${PROJECT_NAME} PRIVATE LibFVR)
