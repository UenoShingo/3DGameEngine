add_library(Engine STATIC
    Core/Engine.cpp
    Core/Engine.h

    Core/Application/Application.cpp
    Core/Application/Application.h

    Core/Logging/Logger.cpp
    Core/Logging/Logger.h

    Core/Debug/Assert.h
    Core/Debug/HResult.h

    Core/Window/Window.cpp
    Core/Window/Window.h
    Core/Window/WindowEvent.h

    Graphics/DirectX12/GraphicsAdapter.cpp
    Graphics/DirectX12/GraphicsAdapter.h

    Graphics/DirectX12/D3D12Device.cpp
    Graphics/DirectX12/D3D12Device.h

    Graphics/DirectX12/FrameResource.h

    Graphics/DirectX12/GpuFence.cpp
    Graphics/DirectX12/GpuFence.h

    Graphics/DirectX12/GraphicsContext.cpp
    Graphics/DirectX12/GraphicsContext.h

    Graphics/DirectX12/SwapChain.cpp
    Graphics/DirectX12/SwapChain.h

    Graphics/Renderer/Renderer.cpp
    Graphics/Renderer/Renderer.h

    Graphics/Shader/CompiledShader.h
    Graphics/Shader/ShaderCompiler.cpp
    Graphics/Shader/ShaderCompiler.h
)

target_compile_features(Engine
    PUBLIC cxx_std_20
)

target_include_directories(Engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${DXC_INCLUDE_DIR}"
)

target_link_libraries(Engine
    PUBLIC
        spdlog::spdlog_header_only
        user32
        gdi32
        dxgi
        d3d12
        "${DXC_LIBRARY}"
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
        MYGAMEENGINE_SHADER_DIR="${PROJECT_SOURCE_DIR}/Shaders"

        "$<$<CONFIG:Debug>:_DEBUG>"
        "$<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>"
        "$<$<NOT:$<CONFIG:Debug>>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>"
)
