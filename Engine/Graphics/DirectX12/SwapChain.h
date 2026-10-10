#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <Windows.h>

#include <array>
#include <cstdint>

namespace MyGameEngine
{
    class GraphicsAdapter;
    class D3D12Device;
    class GraphicsContext;

    struct SwapChainProperties
    {
        std::uint32_t Width = 1280;
        std::uint32_t Height = 720;

        DXGI_FORMAT Format =
            DXGI_FORMAT_R8G8B8A8_UNORM;
    };

    class SwapChain final
    {
    public:
        static constexpr UINT BufferCount = 2;

        SwapChain() = default;
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        SwapChain(SwapChain&&) = delete;
        SwapChain& operator=(SwapChain&&) = delete;

        [[nodiscard]]
        bool Initialize(
            const GraphicsAdapter& adapter,
            const D3D12Device& device,
            const GraphicsContext& context,
            HWND hwnd,
            const SwapChainProperties& properties
        );

        void Shutdown() noexcept;

        [[nodiscard]]
        bool IsValid() const noexcept;

        [[nodiscard]]
        UINT GetCurrentBackBufferIndex() const noexcept;

        [[nodiscard]]
        ID3D12Resource* GetBackBuffer(
            UINT index
        ) const noexcept;

        [[nodiscard]]
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(
            UINT index
        ) const noexcept;

        [[nodiscard]]
        IDXGISwapChain4* GetNativeSwapChain()
            const noexcept;

        [[nodiscard]]
        std::uint32_t GetWidth() const noexcept;

        [[nodiscard]]
        std::uint32_t GetHeight() const noexcept;

    private:
        [[nodiscard]]
        bool CreateRenderTargetViews(
            ID3D12Device* device
        );

        Microsoft::WRL::ComPtr<IDXGISwapChain4>
            m_SwapChain;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
            m_RTVHeap;

        std::array<
            Microsoft::WRL::ComPtr<ID3D12Resource>,
            BufferCount
        > m_BackBuffers{};

        UINT m_RTVDescriptorSize = 0;

        std::uint32_t m_Width = 0;
        std::uint32_t m_Height = 0;

        DXGI_FORMAT m_Format =
            DXGI_FORMAT_UNKNOWN;
    };
}
