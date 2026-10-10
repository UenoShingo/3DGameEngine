#include "SampleGameApplication.h"

#include "Core/Logging/Logger.h"

#include <Windows.h>

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

    m_FrameNumber = 0;

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

    const auto frameIndex =
        static_cast<std::uint32_t>(
            m_FrameNumber %
            MyGameEngine::GraphicsContext::FrameCount
        );

    if (!m_GraphicsContext.BeginFrame(frameIndex))
    {
        ENGINE_LOG_CRITICAL(
            "SampleGame BeginFrame failed"
        );
        RequestExit();
        return;
    }

    // Commit 012 will add rendering commands.
    if (!m_GraphicsContext.EndFrame())
    {
        ENGINE_LOG_CRITICAL(
            "SampleGame EndFrame failed"
        );
        RequestExit();
        return;
    }

    ++m_FrameNumber;

    Sleep(16);
}

void SampleGameApplication::OnShutdown()
{
    ENGINE_LOG_INFO("SampleGame shutting down");

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
        ENGINE_LOG_INFO(
            "SampleGame resized: {}x{}",
            event.Width,
            event.Height
        );
        break;

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
