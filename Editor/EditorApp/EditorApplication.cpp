#include "EditorApplication.h"

#include <iostream>

bool EditorApplication::OnInitialize()
{
    std::cout << "[Editor] Initialize\n";
    return true;
}

void EditorApplication::OnUpdate()
{
    std::cout << "[Editor] Update: "
              << ++m_FrameCount << '\n';

    // Commit 005以降でウィンドウイベントを処理する。
    // 現段階では3フレームで終了する。
    if (m_FrameCount >= 3)
    {
        RequestExit();
    }
}

void EditorApplication::OnShutdown()
{
    std::cout << "[Editor] Shutdown\n";
}
