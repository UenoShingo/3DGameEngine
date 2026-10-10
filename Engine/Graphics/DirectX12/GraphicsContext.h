#pragma once

#include <d3d12.h>
#include <wrl/client.h>

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
        GraphicsContext() = default;
        ~GraphicsContext();

        GraphicsContext(const GraphicsContext&) = delete;
        GraphicsContext& operator=(const GraphicsContext&) = delete;

        GraphicsContext(GraphicsContext&&) = delete;
        GraphicsContext& operator=(GraphicsContext&&) = delete;

        [[nodiscard]]
        bool Initialize(const D3D12Device& device);

        [[nodiscard]]
        bool Begin();

        [[nodiscard]]
        bool Close();

        [[nodiscard]]
        bool Execute();

        void Shutdown() noexcept;

        [[nodiscard]]
        bool IsValid() const noexcept;

        [[nodiscard]]
        CommandContextState GetState() const noexcept;

        [[nodiscard]]
        ID3D12CommandQueue*
        GetCommandQueue() const noexcept;

        [[nodiscard]]
        ID3D12CommandAllocator*
        GetCommandAllocator() const noexcept;

        [[nodiscard]]
        ID3D12GraphicsCommandList*
        GetCommandList() const noexcept;

    private:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue>
            m_CommandQueue;

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator>
            m_CommandAllocator;

        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>
            m_CommandList;

        CommandContextState m_State =
            CommandContextState::Uninitialized;
    };
}
