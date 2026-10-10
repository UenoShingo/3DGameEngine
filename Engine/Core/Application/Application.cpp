#include "Application.h"

namespace MyGameEngine
{
    int Application::Run()
    {
        if (m_IsRunning)
        {
            return 1;
        }

        if (!OnInitialize())
        {
            return 1;
        }

        m_IsRunning = true;

        while (m_IsRunning)
        {
            OnUpdate();
        }

        OnShutdown();

        return 0;
    }

    void Application::RequestExit()
    {
        m_IsRunning = false;
    }

    bool Application::IsRunning() const noexcept
    {
        return m_IsRunning;
    }
}
