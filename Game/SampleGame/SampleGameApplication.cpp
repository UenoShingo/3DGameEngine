#include "SampleGameApplication.h"

#include "Core/Debug/Assert.h"
#include "Core/Logging/Logger.h"

bool SampleGameApplication::OnInitialize()
{
    ENGINE_LOG_INFO("SampleGame initializing");

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

    m_Window.Show();

    ENGINE_ASSERT(
        m_Window.IsValid(),
        "SampleGame window must be valid"
    );

    m_FrameCount = 0;

    ENGINE_LOG_INFO("SampleGame initialized");

    return true;
}

void SampleGameApplication::OnUpdate()
{
    ++m_FrameCount;

    ENGINE_LOG_DEBUG(
        "SampleGame frame: {}",
        m_FrameCount
    );

    Sleep(700);

    if (m_FrameCount >= 3)
    {
        RequestExit();
    }
}

void SampleGameApplication::OnShutdown()
{
    ENGINE_LOG_INFO("SampleGame shutting down");

    m_Window.Destroy();
}
