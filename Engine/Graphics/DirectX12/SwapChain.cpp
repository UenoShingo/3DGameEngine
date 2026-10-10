#include "SwapChain.h"

#include "Core/Debug/HResult.h"
#include "Core/Logging/Logger.h"

#include "Graphics/DirectX12/GraphicsAdapter.h"
#include "Graphics/DirectX12/D3D12Device.h"
#include "Graphics/DirectX12/GraphicsContext.h"

namespace MyGameEngine
{
    SwapChain::~SwapChain()
    {
        Shutdown();
    }

    bool SwapChain::Initialize(
        const GraphicsAdapter& adapter,
        const D3D12Device& device,
        const GraphicsContext& context,
        HWND hwnd,
        const SwapChainProperties& properties)
    {
        if (IsValid())
        {
            ENGINE_LOG_ERROR(
                "SwapChain already initialized"
            );
            return false;
        }

        if (!adapter.IsValid() ||
            !device.IsValid() ||
            !context.IsValid() ||
            !IsWindow(hwnd) ||
            properties.Width == 0 ||
            properties.Height == 0)
        {
            ENGINE_LOG_ERROR(
                "Invalid SwapChain initialization arguments"
            );
            return false;
        }

        RECT clientRect{};

        if (!GetClientRect(hwnd, &clientRect))
        {
            ENGINE_LOG_ERROR(
                "GetClientRect failed: {}",
                GetLastError()
            );
            return false;
        }

        const auto clientWidth =
            clientRect.right - clientRect.left;

        const auto clientHeight =
            clientRect.bottom - clientRect.top;

        if (clientWidth <= 0 || clientHeight <= 0)
        {
            ENGINE_LOG_ERROR(
                "Window client area is zero-sized"
            );
            return false;
        }

        m_Width = static_cast<std::uint32_t>(
            clientWidth
        );

        m_Height = static_cast<std::uint32_t>(
            clientHeight
        );

        m_Format = properties.Format;

        DXGI_SWAP_CHAIN_DESC1 description{};
        description.Width = m_Width;
        description.Height = m_Height;
        description.Format = m_Format;
        description.Stereo = FALSE;
        description.SampleDesc.Count = 1;
        description.SampleDesc.Quality = 0;
        description.BufferUsage =
            DXGI_USAGE_RENDER_TARGET_OUTPUT;
        description.BufferCount = BufferCount;
        description.Scaling = DXGI_SCALING_STRETCH;
        description.SwapEffect =
            DXGI_SWAP_EFFECT_FLIP_DISCARD;
        description.AlphaMode =
            DXGI_ALPHA_MODE_UNSPECIFIED;
        description.Flags = 0;

        Microsoft::WRL::ComPtr<IDXGISwapChain1>
            swapChain1;

        const HRESULT hr =
            adapter.GetFactory()->CreateSwapChainForHwnd(
                context.GetCommandQueue(),
                hwnd,
                &description,
                nullptr,
                nullptr,
                swapChain1.GetAddressOf()
            );

        if (!ENGINE_CHECK_HR(hr))
        {
            Shutdown();
            return false;
        }

        if (!ENGINE_CHECK_HR(
                swapChain1.As(&m_SwapChain)))
        {
            Shutdown();
            return false;
        }

        if (!ENGINE_CHECK_HR(
                adapter.GetFactory()->MakeWindowAssociation(
                    hwnd,
                    DXGI_MWA_NO_ALT_ENTER)))
        {
            ENGINE_LOG_WARN(
                "Could not disable DXGI Alt+Enter handling"
            );
        }

        if (!CreateRenderTargetViews(device.GetDevice()))
        {
            Shutdown();
            return false;
        }

        ENGINE_LOG_INFO(
            "SwapChain initialized: {}x{}, {} buffers",
            m_Width,
            m_Height,
            BufferCount
        );

        return true;
    }

    bool SwapChain::CreateRenderTargetViews(
        ID3D12Device* device)
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heapDesc.NumDescriptors = BufferCount;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = 0;

        if (!ENGINE_CHECK_HR(
                device->CreateDescriptorHeap(
                    &heapDesc,
                    IID_PPV_ARGS(
                        m_RTVHeap.GetAddressOf()
                    ))))
        {
            return false;
        }

        m_RTVDescriptorSize =
            device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV
            );

        D3D12_CPU_DESCRIPTOR_HANDLE handle =
            m_RTVHeap->GetCPUDescriptorHandleForHeapStart();

        for (UINT index = 0;
             index < BufferCount;
             ++index)
        {
            if (!ENGINE_CHECK_HR(
                    m_SwapChain->GetBuffer(
                        index,
                        IID_PPV_ARGS(
                            m_BackBuffers[index]
                                .GetAddressOf()
                        ))))
            {
                return false;
            }

            device->CreateRenderTargetView(
                m_BackBuffers[index].Get(),
                nullptr,
                handle
            );

            handle.ptr += m_RTVDescriptorSize;

            ENGINE_LOG_DEBUG(
                "RTV created for back buffer {}",
                index
            );
        }

        return true;
    }

    void SwapChain::Shutdown() noexcept
    {
        for (auto& buffer : m_BackBuffers)
        {
            buffer.Reset();
        }

        m_RTVHeap.Reset();
        m_SwapChain.Reset();

        m_RTVDescriptorSize = 0;
        m_Width = 0;
        m_Height = 0;
        m_Format = DXGI_FORMAT_UNKNOWN;
    }

    bool SwapChain::IsValid() const noexcept
    {
        if (!m_SwapChain || !m_RTVHeap)
        {
            return false;
        }

        for (const auto& buffer : m_BackBuffers)
        {
            if (!buffer)
            {
                return false;
            }
        }

        return true;
    }

    UINT SwapChain::GetCurrentBackBufferIndex() const noexcept
    {
        return m_SwapChain
            ? m_SwapChain->GetCurrentBackBufferIndex()
            : 0;
    }

    ID3D12Resource* SwapChain::GetBackBuffer(
        UINT index) const noexcept
    {
        if (index >= BufferCount)
        {
            return nullptr;
        }

        return m_BackBuffers[index].Get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE
    SwapChain::GetRTVHandle(UINT index) const noexcept
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle{};

        if (!m_RTVHeap || index >= BufferCount)
        {
            return handle;
        }

        handle =
            m_RTVHeap->GetCPUDescriptorHandleForHeapStart();

        handle.ptr +=
            static_cast<SIZE_T>(index) *
            m_RTVDescriptorSize;

        return handle;
    }

    IDXGISwapChain4*
    SwapChain::GetNativeSwapChain() const noexcept
    {
        return m_SwapChain.Get();
    }

    std::uint32_t SwapChain::GetWidth() const noexcept
    {
        return m_Width;
    }

    std::uint32_t SwapChain::GetHeight() const noexcept
    {
        return m_Height;
    }
}
