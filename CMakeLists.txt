cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -g")

project(MP3Cleaner)

include_directories(${CMAKE_SOURCE_DIR}/lib/tag/include/taglib)
link_directories(${CMAKE_SOURCE_DIR}/lib/tag/lib/)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} tag z)
