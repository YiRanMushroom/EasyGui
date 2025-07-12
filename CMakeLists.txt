cmake_minimum_required(VERSION 4.0)
project(EasyGui)
set(PROJECT_NAME EasyGui)

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} STATIC)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE MODULE_FILES src/*.ixx src/*.cppm)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        BASE_DIRS src
        FILES ${MODULE_FILES})