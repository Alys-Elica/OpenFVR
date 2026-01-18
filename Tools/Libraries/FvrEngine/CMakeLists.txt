cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME FvrEngine)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} SHARED
    libfvrengine_globals.h

    base/miniaudio.h

    engine/audio.h
    engine/audio.cpp
    engine/eventmanager.h
    engine/eventmanager.cpp
    
    engine.h
    engine.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBFVRENGINE_EXPORTS)

# FFMPEG
# find_package(FFMPEG REQUIRED)
# target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
# target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
# target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    PkgConfig::LIBAV
)

# SDL3
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)

# ofnx
add_dependencies(${PROJECT_NAME} ofnx)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ofnx)
