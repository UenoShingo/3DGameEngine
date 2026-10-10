#include "EditorApplication.h"

#include "Core/Debug/Assert.h"
#include "Core/Logging/Logger.h"

bool EditorApplication::OnInitialize()
{
    ENGINE_LOG_INFO("Editor initializing");

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

    m_Window.Show();

    ENGINE_ASSERT(
        m_Window.IsValid(),
        "Editor window must be valid"
    );

    m_FrameCount = 0;

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

    // Temporary visualization for Commit 005.
    // Commit 006 introduces the Win32 message loop.
    Sleep(700);

    if (m_FrameCount >= 3)
    {
        RequestExit();
    }
}

void EditorApplication::OnShutdown()
{
    ENGINE_LOG_INFO("Editor shutting down");

    m_Window.Destroy();
}
