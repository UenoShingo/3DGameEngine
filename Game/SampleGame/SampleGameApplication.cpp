#include "SampleGameApplication.h"

#include "Core/Logging/Logger.h"

#include <Windows.h>

#include <filesystem>

bool SampleGameApplication::OnInitialize()
{
    ENGINE_LOG_INFO("SampleGame initializing");

    m_Window.SetEventCallback(
        [this](const MyGameEngine::WindowEvent& event)
        {
            OnWindowEvent(event);
        }
    );

    MyGameEngine::WindowProperties properties;
    properties.Title = L"MyGameEngine SampleGame";
    properties.Width = 1280;
    properties.Height = 720;

    if (!m_Window.Create(properties))
    {
        ENGINE_LOG_ERROR(
            "Failed to create SampleGame window"
        );
        return false;
    }

    MyGameEngine::D3D12DeviceOptions options{};

#ifdef _DEBUG
    options.EnableDebugLayer = true;
    options.EnableGpuValidation = false;
#else
    options.EnableDebugLayer = false;
    options.EnableGpuValidation = false;
#endif

    if (!MyGameEngine::D3D12Device::
            ConfigureDebugLayer(options))
    {
        return false;
    }

    if (!m_GraphicsAdapter.Initialize())
    {
        ENGINE_LOG_ERROR(
            "Failed to initialize SampleGame GPU"
        );
        return false;
    }

    if (!m_D3D12Device.Initialize(m_GraphicsAdapter))
    {
        ENGINE_LOG_ERROR(
            "SampleGame D3D12Device initialization failed"
        );
        return false;
    }

    if (!m_GraphicsContext.Initialize(m_D3D12Device))
    {
        ENGINE_LOG_ERROR(
            "SampleGame GraphicsContext initialization failed"
        );
        return false;
    }

    MyGameEngine::SwapChainProperties swapChainProperties{};
    swapChainProperties.Width = m_Window.GetWidth();
    swapChainProperties.Height = m_Window.GetHeight();
    swapChainProperties.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    if (!m_SwapChain.Initialize(
            m_GraphicsAdapter,
            m_D3D12Device,
            m_GraphicsContext,
            m_Window.GetHandle(),
            swapChainProperties))
    {
        ENGINE_LOG_ERROR(
            "Failed to initialize SampleGame SwapChain"
        );
        return false;
    }

    if (!m_ShaderCompiler.Initialize())
    {
        ENGINE_LOG_ERROR(
            "SampleGame ShaderCompiler initialization failed"
        );
        return false;
    }

    const auto shaderPath =
        std::filesystem::path(MYGAMEENGINE_SHADER_DIR) /
        "Basic.hlsl";

    MyGameEngine::ShaderCompileOptions vsOptions{};
    vsOptions.FilePath = shaderPath;
    vsOptions.EntryPoint = L"VSMain";
    vsOptions.Stage = MyGameEngine::ShaderStage::Vertex;

#ifdef _DEBUG
    vsOptions.EnableDebugInfo = true;
    vsOptions.EnableOptimization = false;
#endif

    if (!m_ShaderCompiler.Compile(vsOptions, m_VertexShader))
    {
        ENGINE_LOG_ERROR(
            "SampleGame Vertex Shader compilation failed"
        );
        return false;
    }

    MyGameEngine::ShaderCompileOptions psOptions{};
    psOptions.FilePath = shaderPath;
    psOptions.EntryPoint = L"PSMain";
    psOptions.Stage = MyGameEngine::ShaderStage::Pixel;

#ifdef _DEBUG
    psOptions.EnableDebugInfo = true;
    psOptions.EnableOptimization = false;
#endif

    if (!m_ShaderCompiler.Compile(psOptions, m_PixelShader))
    {
        ENGINE_LOG_ERROR(
            "SampleGame Pixel Shader compilation failed"
        );
        return false;
    }

    ENGINE_LOG_INFO(
        "SampleGame shaders compiled successfully"
    );

    m_Renderer.SetClearColor(0.12f, 0.20f, 0.16f, 1.0f);
    m_Renderer.SetVSync(true);

    m_PendingWidth = 0;
    m_PendingHeight = 0;
    m_ResizePending = false;
    m_IsMinimized = false;

    ENGINE_LOG_INFO("SampleGame Renderer initialized");

    m_Window.Show();

    ENGINE_LOG_INFO("SampleGame initialized");

    return true;
}

void SampleGameApplication::OnUpdate()
{
    if (!m_Window.ProcessMessages())
    {
        RequestExit();
        return;
    }

    if (!m_Window.IsValid())
    {
        RequestExit();
        return;
    }

    if (m_IsMinimized)
    {
        Sleep(16);
        return;
    }

    if (m_ResizePending)
    {
        RECT clientRect{};

        if (!GetClientRect(m_Window.GetHandle(), &clientRect))
        {
            ENGINE_LOG_ERROR(
                "SampleGame GetClientRect failed"
            );
            RequestExit();
            return;
        }

        const auto width = static_cast<std::uint32_t>(
            clientRect.right - clientRect.left
        );
        const auto height = static_cast<std::uint32_t>(
            clientRect.bottom - clientRect.top
        );

        if (width == 0 || height == 0)
        {
            Sleep(16);
            return;
        }

        if (!m_GraphicsContext.PrepareForResize())
        {
            ENGINE_LOG_CRITICAL(
                "SampleGame GPU resize synchronization failed"
            );
            RequestExit();
            return;
        }

        if (!m_SwapChain.Resize(
                m_D3D12Device.GetDevice(),
                width,
                height))
        {
            ENGINE_LOG_CRITICAL(
                "SampleGame SwapChain resize failed"
            );
            RequestExit();
            return;
        }

        m_ResizePending = false;
    }

    if (!m_Renderer.RenderFrame(
            m_GraphicsContext,
            m_SwapChain))
    {
        ENGINE_LOG_CRITICAL("SampleGame rendering failed");
        RequestExit();
    }
}

void SampleGameApplication::OnShutdown()
{
    ENGINE_LOG_INFO("SampleGame shutting down");

    m_VertexShader.Bytecode.Reset();
    m_PixelShader.Bytecode.Reset();
    m_ShaderCompiler.Shutdown();

    if (!m_GraphicsContext.Shutdown())
    {
        ENGINE_LOG_CRITICAL(
            "SampleGame GPU synchronization failed"
        );
    }

    m_SwapChain.Shutdown();
    m_D3D12Device.Shutdown();
    m_GraphicsAdapter.Shutdown();
    m_Window.Destroy();

    ENGINE_LOG_INFO("SampleGame shutdown completed");
}

void SampleGameApplication::OnWindowEvent(
    const MyGameEngine::WindowEvent& event)
{
    using MyGameEngine::WindowEventType;

    switch (event.Type)
    {
    case WindowEventType::Closed:
        ENGINE_LOG_INFO("SampleGame window closed");
        RequestExit();
        break;

    case WindowEventType::Resized:
    {
        m_PendingWidth = event.Width;
        m_PendingHeight = event.Height;
        m_IsMinimized = event.Width == 0 || event.Height == 0;

        if (!m_IsMinimized)
        {
            m_ResizePending = true;
        }

        ENGINE_LOG_INFO(
            "SampleGame resize requested: {}x{}",
            event.Width,
            event.Height
        );
        break;
    }

    case WindowEventType::FocusGained:
        ENGINE_LOG_INFO("SampleGame focus gained");
        break;

    case WindowEventType::FocusLost:
        ENGINE_LOG_INFO("SampleGame focus lost");
        break;

    case WindowEventType::KeyDown:
        if (!event.IsRepeat)
        {
            ENGINE_LOG_DEBUG(
                "SampleGame key down: {}",
                event.KeyCode
            );
        }
        break;

    case WindowEventType::KeyUp:
        ENGINE_LOG_DEBUG(
            "SampleGame key up: {}",
            event.KeyCode
        );
        break;
    }
}
