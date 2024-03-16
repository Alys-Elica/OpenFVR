cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME Tools)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/out/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/out/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/out/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/out/${CMAKE_BUILD_TYPE}/bin)

add_subdirectory(ArnVitConverter)
add_subdirectory(DatExtractor)
add_subdirectory(MtrConverter)
add_subdirectory(PakConverter)
add_subdirectory(VrConverter)
