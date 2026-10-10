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

    m_Renderer.SetClearColor(0.09f, 0.14f, 0.22f, 1.0f);
    m_Renderer.SetVSync(true);

    m_PendingWidth = 0;
    m_PendingHeight = 0;
    m_ResizePending = false;
    m_IsMinimized = false;

    ENGINE_LOG_INFO("Editor Renderer initialized");

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
                "GetClientRect failed: {}",
                GetLastError()
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
                "GPU synchronization before resize failed"
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
                "Editor SwapChain resize failed"
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
        ENGINE_LOG_CRITICAL(
            "Editor frame rendering failed"
        );
        RequestExit();
    }
}

void EditorApplication::OnShutdown()
{
    ENGINE_LOG_INFO("Editor shutting down");

    if (!m_GraphicsContext.Shutdown())
    {
        ENGINE_LOG_CRITICAL(
            "GPU shutdown synchronization failed"
        );
    }

    m_SwapChain.Shutdown();
    m_D3D12Device.Shutdown();
    m_GraphicsAdapter.Shutdown();
    m_Window.Destroy();

    ENGINE_LOG_INFO("Editor shutdown completed");
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
    {
        m_PendingWidth = event.Width;
        m_PendingHeight = event.Height;
        m_IsMinimized = event.Width == 0 || event.Height == 0;

        if (!m_IsMinimized)
        {
            m_ResizePending = true;
        }

        ENGINE_LOG_INFO(
            "Editor resize requested: {}x{}",
            event.Width,
            event.Height
        );
        break;
    }

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
