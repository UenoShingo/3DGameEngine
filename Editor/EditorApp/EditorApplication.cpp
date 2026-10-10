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
            "Editor resized: {}x{}",
            event.Width,
            event.Height
        );
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
