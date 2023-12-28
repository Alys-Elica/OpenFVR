cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME VrConverter)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/out/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/out/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/out/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/out/${CMAKE_BUILD_TYPE}/bin)

set(PROJECT_FILE_HEADERS
    dct.h
    file.h
    filevr.h
    fnxvr.h
    frame.h
)

set(PROJECT_FILE_SOURCES
    dct.cpp
    file.cpp
    filevr.cpp
    fnxvr.cpp
    frame.cpp

    main.cpp
)

add_executable(${PROJECT_NAME}
    ${PROJECT_FILE_HEADERS}
    ${PROJECT_FILE_SOURCES}
)

# Boost
find_package(Boost REQUIRED COMPONENTS program_options)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost Boost::program_options)

# LibPNG
find_package(PNG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)

# LibWebP
find_package(WebP CONFIG REQUIRED)
# Basic usage
target_link_libraries(${PROJECT_NAME} PRIVATE WebP::webp WebP::webpdecoder WebP::webpdemux)
# For manipulating the WebP graphics format container (port feature libwebpmux)
target_link_libraries(${PROJECT_NAME} PRIVATE WebP::libwebpmux)
# For sharp RGB to YUV conversion
#target_link_libraries(${PROJECT_NAME} PRIVATE WebP::sharpyuv)

# SDL2
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)
