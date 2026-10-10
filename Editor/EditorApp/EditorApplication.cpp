#include "EditorApplication.h"

#include "Core/Logging/Logger.h"

#include <Windows.h>

bool EditorApplication::OnInitialize()
{
    ENGINE_LOG_INFO("Editor initializing");

    m_Window.SetEventCallback(
        [this](const MyGameEngine::WindowEvent& event)
        {
            OnWindowEvent(event);
        }
    );

    MyGameEngine::WindowProperties properties;
    properties.Title = L"MyGameEngine Editor";
    properties.Width = 1280;
    properties.Height = 720;

    if (!m_Window.Create(properties))
    {
        ENGINE_LOG_ERROR(
            "Failed to create Editor window"
        );
        return false;
    }

    MyGameEngine::D3D12DeviceOptions deviceOptions{};

#ifdef _DEBUG
    deviceOptions.EnableDebugLayer = true;
    deviceOptions.EnableGpuValidation = false;
#else
    deviceOptions.EnableDebugLayer = false;
    deviceOptions.EnableGpuValidation = false;
#endif

    // Must run before GraphicsAdapter initialization.
    if (!MyGameEngine::D3D12Device::
            ConfigureDebugLayer(deviceOptions))
    {
        return false;
    }

    if (!m_GraphicsAdapter.Initialize())
    {
        ENGINE_LOG_ERROR(
            "GraphicsAdapter initialization failed"
        );
        return false;
    }

    if (!m_D3D12Device.Initialize(m_GraphicsAdapter))
    {
        ENGINE_LOG_ERROR(
            "D3D12Device initialization failed"
        );
        return false;
    }

    if (!m_GraphicsContext.Initialize(m_D3D12Device))
    {
        ENGINE_LOG_ERROR(
            "GraphicsContext initialization failed"
        );
        return false;
    }

    // Commit 009: record and submit one empty command list.
    if (!m_GraphicsContext.Begin())
    {
        return false;
    }

    if (!m_GraphicsContext.Close())
    {
        return false;
    }

    if (!m_GraphicsContext.Execute())
    {
        return false;
    }

    MyGameEngine::SwapChainProperties swapChainProperties{};
    swapChainProperties.Width = m_Window.GetWidth();
    swapChainProperties.Height = m_Window.GetHeight();
    swapChainProperties.Format =
        DXGI_FORMAT_R8G8B8A8_UNORM;

    if (!m_SwapChain.Initialize(
            m_GraphicsAdapter,
            m_D3D12Device,
            m_GraphicsContext,
            m_Window.GetHandle(),
            swapChainProperties))
    {
        ENGINE_LOG_ERROR(
            "Failed to initialize Editor SwapChain"
        );
        return false;
    }

    ENGINE_LOG_INFO(
        "Editor SwapChain ready: {}x{}",
        m_SwapChain.GetWidth(),
        m_SwapChain.GetHeight()
    );

    m_Window.Show();

    ENGINE_LOG_INFO("Editor initialized");
    return true;
}

void EditorApplication::OnUpdate()
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

    // Temporary idle delay.
    // Replace with frame timing in later commits.
    Sleep(16);
}

void EditorApplication::OnShutdown()
{
    ENGINE_LOG_INFO("Editor shutting down");

    m_SwapChain.Shutdown();
    m_GraphicsContext.Shutdown();
    m_D3D12Device.Shutdown();
    m_GraphicsAdapter.Shutdown();
    m_Window.Destroy();
}

void EditorApplication::OnWindowEvent(
    const MyGameEngine::WindowEvent& event)
{
    using MyGameEngine::WindowEventType;

    switch (event.Type)
    {
    case WindowEventType::Closed:
        ENGINE_LOG_INFO("Editor window closed");
        RequestExit();
        break;

    case WindowEventType::Resized:
        ENGINE_LOG_INFO(
            "Editor window resized: {}x{}",
            event.Width,
            event.Height
        );
        // SwapChain::Resize() will be added after GPU
        // synchronization is available.
        break;

    case WindowEventType::FocusGained:
        ENGINE_LOG_INFO("Editor focus gained");
        break;

    case WindowEventType::FocusLost:
        ENGINE_LOG_INFO("Editor focus lost");
        break;

    case WindowEventType::KeyDown:
        if (!event.IsRepeat)
        {
            ENGINE_LOG_DEBUG(
                "Editor key down: {}",
                event.KeyCode
            );
        }
        break;

    case WindowEventType::KeyUp:
        ENGINE_LOG_DEBUG(
            "Editor key up: {}",
            event.KeyCode
        );
        break;
    }
}
