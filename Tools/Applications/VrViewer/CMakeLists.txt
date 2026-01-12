cmake_minimum_required(VERSION 3.25)

set(PROJECT_NAME VrViewer)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
    WIN32
    
    main.cpp
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()

# SDL3
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# ofnx
add_dependencies(${PROJECT_NAME} ofnx)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ofnx)
