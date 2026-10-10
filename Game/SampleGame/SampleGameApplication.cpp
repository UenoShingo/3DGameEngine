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

    if (!m_GraphicsAdapter.Initialize())
    {
        ENGINE_LOG_ERROR(
            "Failed to initialize SampleGame GPU"
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

    Sleep(16);
}

void SampleGameApplication::OnShutdown()
{
    ENGINE_LOG_INFO("SampleGame shutting down");

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
