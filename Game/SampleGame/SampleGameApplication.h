#pragma once

#include "Core/Application/Application.h"
#include "Core/Window/Window.h"
#include "Core/Window/WindowEvent.h"

#include "Graphics/DirectX12/GraphicsAdapter.h"

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
    void OnWindowEvent(
        const MyGameEngine::WindowEvent& event
    );

    MyGameEngine::Window m_Window;
    MyGameEngine::GraphicsAdapter m_GraphicsAdapter;
};
