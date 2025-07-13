cmake_minimum_required(VERSION 4.0)
project(EasyGui)
set(PROJECT_NAME EasyGui)

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} STATIC)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE MODULE_FILES src/*.ixx src/*.cppm)
file(GLOB_RECURSE HEADER_FILES src/*.h src/*.hpp)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules
        TYPE CXX_MODULES
        BASE_DIRS src
        FILES ${MODULE_FILES})

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)

set(SDL_VULKAN ON)
set(SDL_STATIC ON)
set(SDL_SHARED OFF)
add_subdirectory(vendor/SDL3)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3)

set(IMGUI_DEFINE_MATH_OPERATORS ON)
set(IMGUI_BACKEND_RENDERER "vulkan" CACHE STRING "ImGui backend renderer")
set(IMGUI_BACKEND_PLATFORM "sdl3" CACHE STRING "ImGui backend platform")
add_subdirectory(vendor/imgui)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)

add_subdirectory(vendor/soloud)
target_link_libraries(${PROJECT_NAME} PUBLIC SoLoud)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        "VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1"
        "VK_USE_PLATFORM_WIN32_KHR=1"
        "VULKAN_HPP_NO_EXCEPTIONS=1"
        "VULKAN_HPP_RAII_NO_EXCEPTIONS=1"
        "VULKAN_HPP_CPP_VERSION=23"
)

target_include_directories(${PROJECT_NAME} PUBLIC
        src)