cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME FvrEngine)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_FILE_HEADERS
    base/miniaudio.h

    engine/audio.h
    engine/script.h
    
    libfvrengine_globals.h
    engine.h
)

set(PROJECT_FILE_SOURCES
    engine/audio.cpp
    engine/script.cpp
    
    engine.cpp
)

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_FILE_HEADERS}
    ${PROJECT_FILE_SOURCES}
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

# ofnx
add_dependencies(${PROJECT_NAME} ofnx)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ofnx)
