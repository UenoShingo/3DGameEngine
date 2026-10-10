add_library(Engine STATIC
    Core/Engine.cpp
    Core/Engine.h
    Core/Application/Application.cpp
    Core/Application/Application.h
)

target_compile_features(Engine
    PUBLIC cxx_std_20
)

target_include_directories(Engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(Engine
    PRIVATE /W4 /permissive- /utf-8
)
