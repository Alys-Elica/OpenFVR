cmake_minimum_required(VERSION 3.14.0)

set(PROJECT_NAME LibFVR)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_FILE_HEADERS
    libfvr_globals.h

    fvr/image.h
    fvr/file.h

    fvr_files/fvr_arnvit.h
    fvr_files/fvr_dat.h
    fvr_files/fvr_mtr.h
    fvr_files/fvr_pak.h
    fvr_files/fvr_tst.h
    fvr_files/fvr_vr.h

    internal/dct.h
)

set(PROJECT_FILE_SOURCES
    fvr/file.cpp
    fvr/image.cpp

    fvr_files/fvr_arnvit.cpp
    fvr_files/fvr_dat.cpp
    fvr_files/fvr_mtr.cpp
    fvr_files/fvr_pak.cpp
    fvr_files/fvr_tst.cpp
    fvr_files/fvr_vr.cpp

    internal/dct.cpp
)

add_library(${PROJECT_NAME} SHARED
    ${PROJECT_FILE_HEADERS}
    ${PROJECT_FILE_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_compile_definitions(
    LIBFVR_EXPORTS
)

# LibPNG
find_package(PNG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
