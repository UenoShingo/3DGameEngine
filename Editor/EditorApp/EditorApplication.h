#pragma once

#include "Core/Application/Application.h"

class EditorApplication final : public MyGameEngine::Application
{
public:
    EditorApplication() = default;
    ~EditorApplication() override = default;

protected:
    bool OnInitialize() override;
    void OnUpdate() override;
    void OnShutdown() override;

private:
    unsigned int m_FrameCount = 0;
};
