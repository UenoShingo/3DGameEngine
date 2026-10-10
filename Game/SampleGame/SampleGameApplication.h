#pragma once

#include "Core/Application/Application.h"
#include "Core/Window/Window.h"
#include "Core/Window/WindowEvent.h"

#include "Graphics/DirectX12/GraphicsAdapter.h"
#include "Graphics/DirectX12/D3D12Device.h"
#include "Graphics/DirectX12/GraphicsContext.h"
#include "Graphics/DirectX12/SwapChain.h"

#include <cstdint>

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
    MyGameEngine::D3D12Device m_D3D12Device;
    MyGameEngine::GraphicsContext m_GraphicsContext;
    MyGameEngine::SwapChain m_SwapChain;
    std::uint64_t m_FrameNumber = 0;
};
