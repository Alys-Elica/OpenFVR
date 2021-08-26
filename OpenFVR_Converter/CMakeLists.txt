cmake_minimum_required (VERSION 3.8)

set(PROJECT_NAME Phoenix_Converter)

project(${PROJECT_NAME} LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    Converter/converterpak.cpp
    main.cpp
    
    Converter/converterpak.h
)