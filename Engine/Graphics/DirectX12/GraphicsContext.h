#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <array>
#include <cstdint>

#include "Graphics/DirectX12/FrameResource.h"
#include "Graphics/DirectX12/GpuFence.h"

namespace MyGameEngine
{
    class D3D12Device;

    enum class CommandContextState
    {
        Uninitialized,
        Ready,
        Recording,
        Closed,
        Submitted,
        Failed
    };

    class GraphicsContext final
    {
    public:
        static constexpr std::uint32_t FrameCount = 2;

        GraphicsContext() = default;
        ~GraphicsContext();

        GraphicsContext(const GraphicsContext&) = delete;
        GraphicsContext& operator=(const GraphicsContext&) = delete;

        [[nodiscard]] bool Initialize(const D3D12Device& device);
        [[nodiscard]] bool BeginFrame(std::uint32_t frameIndex);
        [[nodiscard]] bool EndFrame();
        [[nodiscard]] bool WaitForGpu();
        [[nodiscard]] bool PrepareForResize();
        [[nodiscard]] bool Shutdown();

        [[nodiscard]] bool IsValid() const noexcept;
        [[nodiscard]] CommandContextState GetState() const noexcept;
        [[nodiscard]] ID3D12CommandQueue* GetCommandQueue() const noexcept;
        [[nodiscard]] ID3D12GraphicsCommandList* GetCommandList() const noexcept;
        [[nodiscard]] std::uint32_t GetCurrentFrameIndex() const noexcept;
        [[nodiscard]]
        std::uint64_t GetFrameFenceValue(
            std::uint32_t index) const noexcept;

    private:
        Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

        std::array<FrameResource, FrameCount> m_Frames{};
        GpuFence m_Fence;

        std::uint32_t m_CurrentFrameIndex = 0;
        bool m_HasUntrackedSubmission = false;

        CommandContextState m_State =
            CommandContextState::Uninitialized;
    };
}
