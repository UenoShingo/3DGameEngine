#include "SampleGameApplication.h"

#include "Core/Debug/Assert.h"
#include "Core/Logging/Logger.h"

bool SampleGameApplication::OnInitialize()
{
    ENGINE_LOG_INFO("SampleGame initializing");

    m_FrameCount = 0;

    ENGINE_ASSERT(
        m_FrameCount == 0,
        "Frame count must start at zero"
    );

    ENGINE_LOG_INFO("SampleGame initialized");

    return true;
}

void SampleGameApplication::OnUpdate()
{
    ++m_FrameCount;

    ENGINE_LOG_DEBUG(
        "Game frame: {}",
        m_FrameCount
    );

    if (m_FrameCount >= 3)
    {
        ENGINE_LOG_INFO("Game exit requested");
        RequestExit();
    }
}

void SampleGameApplication::OnShutdown()
{
    ENGINE_LOG_INFO("SampleGame shutting down");
}
