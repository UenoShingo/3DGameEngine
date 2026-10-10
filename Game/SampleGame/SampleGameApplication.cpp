#include "SampleGameApplication.h"

#include <iostream>

bool SampleGameApplication::OnInitialize()
{
    std::cout << "[Game] Initialize\n";
    return true;
}

void SampleGameApplication::OnUpdate()
{
    std::cout << "[Game] Update: "
              << ++m_FrameCount << '\n';

    if (m_FrameCount >= 3)
    {
        RequestExit();
    }
}

void SampleGameApplication::OnShutdown()
{
    std::cout << "[Game] Shutdown\n";
}
