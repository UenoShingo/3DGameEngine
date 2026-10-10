#include "Renderer.h"

#include "Core/Debug/HResult.h"
#include "Core/Logging/Logger.h"
#include "Graphics/DirectX12/GraphicsContext.h"
#include "Graphics/DirectX12/SwapChain.h"

#include <d3d12.h>

namespace MyGameEngine
{
    bool Renderer::RenderFrame(
        GraphicsContext& context,
        SwapChain& swapChain)
    {
        if (!context.IsValid() || !swapChain.IsValid())
        {
            ENGINE_LOG_ERROR(
                "Renderer: invalid graphics resources"
            );
            return false;
        }

        if (swapChain.GetWidth() == 0 ||
            swapChain.GetHeight() == 0)
        {
            return true;
        }

        const UINT frameIndex =
            swapChain.GetCurrentBackBufferIndex();

        if (frameIndex >= GraphicsContext::FrameCount)
        {
            ENGINE_LOG_ERROR(
                "Renderer: invalid frame index {}",
                frameIndex
            );
            return false;
        }

        if (!context.BeginFrame(frameIndex))
        {
            ENGINE_LOG_ERROR("Renderer: BeginFrame failed");
            return false;
        }

        ID3D12GraphicsCommandList* commandList =
            context.GetCommandList();

        ID3D12Resource* backBuffer =
            swapChain.GetBackBuffer(frameIndex);

        if (!commandList || !backBuffer)
        {
            ENGINE_LOG_ERROR(
                "Renderer: invalid command list or buffer"
            );
            return false;
        }

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = backBuffer;
        barrier.Transition.Subresource =
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore =
            D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter =
            D3D12_RESOURCE_STATE_RENDER_TARGET;

        commandList->ResourceBarrier(1, &barrier);

        const D3D12_CPU_DESCRIPTOR_HANDLE rtv =
            swapChain.GetRTVHandle(frameIndex);

        commandList->OMSetRenderTargets(
            1,
            &rtv,
            FALSE,
            nullptr
        );

        commandList->ClearRenderTargetView(
            rtv,
            m_ClearColor.data(),
            0,
            nullptr
        );

        barrier.Transition.StateBefore =
            D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter =
            D3D12_RESOURCE_STATE_PRESENT;

        commandList->ResourceBarrier(1, &barrier);

        if (!context.EndFrame())
        {
            ENGINE_LOG_ERROR("Renderer: EndFrame failed");
            return false;
        }

        const UINT syncInterval = m_VSyncEnabled ? 1u : 0u;
        const HRESULT hr =
            swapChain.GetNativeSwapChain()->Present(
                syncInterval,
                0
            );

        if (!ENGINE_CHECK_HR(hr))
        {
            ENGINE_LOG_CRITICAL("Renderer: Present failed");
            return false;
        }

        return true;
    }

    void Renderer::SetClearColor(
        float red,
        float green,
        float blue,
        float alpha) noexcept
    {
        m_ClearColor = { red, green, blue, alpha };
    }

    void Renderer::SetVSync(bool enabled) noexcept
    {
        m_VSyncEnabled = enabled;
    }

    bool Renderer::IsVSyncEnabled() const noexcept
    {
        return m_VSyncEnabled;
    }
}
