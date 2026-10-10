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

# Dependencies
include(FetchContent)

set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(spdlog)

# DirectX Shader Compiler v1.9.2609
set(DXC_ROOT
    "${PROJECT_SOURCE_DIR}/ThirdParty/DXC"
)

set(DXC_INCLUDE_DIR
    "${DXC_ROOT}/include"
)

set(DXC_LIBRARY
    "${DXC_ROOT}/lib/x64/dxcompiler.lib"
)

set(DXC_RUNTIME_DIR
    "${DXC_ROOT}/bin/x64"
)

if(NOT EXISTS "${DXC_INCLUDE_DIR}/dxcapi.h")
    message(FATAL_ERROR "DXC header not found: dxcapi.h")
endif()

if(NOT EXISTS "${DXC_LIBRARY}")
    message(FATAL_ERROR "DXC library not found: dxcompiler.lib")
endif()

if(NOT EXISTS "${DXC_RUNTIME_DIR}/dxcompiler.dll")
    message(FATAL_ERROR "DXC runtime not found: dxcompiler.dll")
endif()

if(NOT EXISTS "${DXC_RUNTIME_DIR}/dxil.dll")
    message(FATAL_ERROR "DXC runtime not found: dxil.dll")
endif()

# Projects
add_subdirectory(Engine)
add_subdirectory(Editor)
add_subdirectory(Game/SampleGame)
