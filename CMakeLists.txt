cmake_minimum_required(VERSION 3.25)

project(MyGameEngine
    VERSION 0.1.0
    LANGUAGES CXX
)

if(NOT WIN32)
    message(FATAL_ERROR "Windows is required.")
endif()

if(NOT MSVC)
    message(FATAL_ERROR "MSVC is required.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

add_subdirectory(Engine)
add_subdirectory(Editor)
add_subdirectory(Game/SampleGame)
