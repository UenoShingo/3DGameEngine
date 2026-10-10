#pragma once

#include "Core/Application/Application.h"
#include "Core/Window/Window.h"
#include "Core/Window/WindowEvent.h"

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
    void OnWindowEvent(
        const MyGameEngine::WindowEvent& event
    );

    MyGameEngine::Window m_Window;
};
