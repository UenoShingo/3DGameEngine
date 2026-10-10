#pragma once

#include "Core/Application/Application.h"

class SampleGameApplication final
    : public MyGameEngine::Application
{
public:
    SampleGameApplication() = default;
    ~SampleGameApplication() override = default;

protected:
    bool OnInitialize() override;
    void OnUpdate() override;
    void OnShutdown() override;

private:
    unsigned int m_FrameCount = 0;
};
