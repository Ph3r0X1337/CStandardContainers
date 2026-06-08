cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 90)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project("CStandardContainers")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

file(GLOB_RECURSE CSC_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp
     ${PROJECT_SOURCE_DIR}/src/*.c)
add_library(csc ${CSC_SOURCES})

file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/*.cpp
     ${PROJECT_SOURCE_DIR}/test/*.c)
add_executable(test ${TEST_SOURCES})
target_include_directories(test PUBLIC ${PROJECT_SOURCE_DIR}/src/Include)
target_link_libraries(test csc)
