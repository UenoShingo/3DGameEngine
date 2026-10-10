#include "EditorApplication.h"

#include "Core/Debug/Assert.h"
#include "Core/Logging/Logger.h"

bool EditorApplication::OnInitialize()
{
    ENGINE_LOG_INFO("Editor initializing");

    m_FrameCount = 0;

    ENGINE_ASSERT(
        m_FrameCount == 0,
        "Frame count must start at zero"
    );

    ENGINE_LOG_INFO("Editor initialized");

    return true;
}

void EditorApplication::OnUpdate()
{
    ++m_FrameCount;

    ENGINE_LOG_DEBUG(
        "Editor frame: {}",
        m_FrameCount
    );

    if (m_FrameCount >= 3)
    {
        ENGINE_LOG_INFO("Editor exit requested");
        RequestExit();
    }
}

void EditorApplication::OnShutdown()
{
    ENGINE_LOG_INFO("Editor shutting down");
}
