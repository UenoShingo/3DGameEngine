#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

namespace MyGameEngine
{
    class GpuFence final
    {
    public:
        GpuFence() = default;
        ~GpuFence();

        GpuFence(const GpuFence&) = delete;
        GpuFence& operator=(const GpuFence&) = delete;

        [[nodiscard]] bool Initialize(ID3D12Device* device);

        [[nodiscard]]
        bool Signal(
            ID3D12CommandQueue* queue,
            std::uint64_t& signaledValue
        );

        [[nodiscard]] bool Wait(std::uint64_t value);
        [[nodiscard]] bool WaitForGpu(ID3D12CommandQueue* queue);
        [[nodiscard]] bool IsComplete(std::uint64_t value) const;
        [[nodiscard]] std::uint64_t GetCompletedValue() const;
        [[nodiscard]] bool IsValid() const noexcept;

        void Shutdown() noexcept;

    private:
        Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
        HANDLE m_Event = nullptr;
        std::uint64_t m_NextFenceValue = 1;
    };
}
