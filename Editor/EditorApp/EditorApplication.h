#pragma once

#include "Core/Application/Application.h"
#include "Core/Window/Window.h"

#include <cstdint>

class EditorApplication final
    : public MyGameEngine::Application
{
public:
    EditorApplication() = default;
    ~EditorApplication() override = default;

protected:
    bool OnInitialize() override;
    void OnUpdate() override;
    void OnShutdown() override;

private:
    MyGameEngine::Window m_Window;

    std::uint32_t m_FrameCount = 0;
};
