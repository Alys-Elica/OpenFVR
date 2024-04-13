cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME FvrEngine)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_FILE_HEADERS
    base/miniaudio.h

    engine/audio.h
    engine/input.h
    engine/render.h
    engine/script.h
    
    libfvrengine_globals.h
    engine.h
)

set(PROJECT_FILE_SOURCES
    engine/audio.cpp
    engine/input.cpp
    engine/render.cpp
    engine/script.cpp
    
    engine.cpp
)

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_FILE_HEADERS}
    ${PROJECT_FILE_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBFVRENGINE_EXPORTS)

target_link_libraries(${PROJECT_NAME} PRIVATE LibFVR)
target_link_libraries(${PROJECT_NAME} PRIVATE FvrRenderer)

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})

# SDL2_image
find_package(SDL2_image REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
