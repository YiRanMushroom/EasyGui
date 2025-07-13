cmake_minimum_required(VERSION 4.0)
project(SoLoud)
set(PROJECT_NAME SoLoud)

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} STATIC)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE MODULE_FILES src/*.ixx src/*.cppm)
file(GLOB_RECURSE HEADER_FILES include/*.h include/*.hpp)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        BASE_DIRS src
        FILES ${MODULE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "WITH_WINMM=1"
)
