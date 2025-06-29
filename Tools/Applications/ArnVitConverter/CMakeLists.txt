cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME ArnVitConverter)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
    main.cpp
)

# ofnx
add_dependencies(${PROJECT_NAME} ofnx)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ofnx)
