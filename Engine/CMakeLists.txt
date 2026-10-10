add_library(Engine STATIC
    Core/Engine.cpp
    Core/Engine.h

    Core/Application/Application.cpp
    Core/Application/Application.h

    Core/Logging/Logger.cpp
    Core/Logging/Logger.h

    Core/Debug/Assert.h

    Core/Window/Window.cpp
    Core/Window/Window.h
    Core/Window/WindowEvent.h
)

target_compile_features(Engine
    PUBLIC cxx_std_20
)

target_include_directories(Engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Engine
    PUBLIC spdlog::spdlog_header_only
    PRIVATE user32 gdi32
)

target_compile_options(Engine
    PRIVATE /W4 /permissive- /utf-8
)

# Compile-time log filtering
target_compile_definitions(Engine
    PUBLIC
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX

        "$<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>"
        "$<$<NOT:$<CONFIG:Debug>>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>"
)
