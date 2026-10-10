#include "GraphicsAdapter.h"

#include "Core/Logging/Logger.h"

#include <Windows.h>

#include <string>

namespace MyGameEngine
{
    namespace
    {
        std::string ToUtf8(const wchar_t* text)
        {
            if (text == nullptr)
            {
                return {};
            }

            const int length = WideCharToMultiByte(
                CP_UTF8,
                0,
                text,
                -1,
                nullptr,
                0,
                nullptr,
                nullptr
            );

            if (length <= 0)
            {
                return {};
            }

            std::string result(
                static_cast<std::size_t>(length),
                '\0'
            );

            if (WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    text,
                    -1,
                    result.data(),
                    length,
                    nullptr,
                    nullptr) == 0)
            {
                return {};
            }

            result.pop_back();
            return result;
        }
    }

    bool GraphicsAdapter::Initialize(bool allowWarp)
    {
        Shutdown();

        UINT factoryFlags = 0;

#ifdef _DEBUG
        factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

        HRESULT hr = CreateDXGIFactory2(
            factoryFlags,
            IID_PPV_ARGS(
                m_Factory.ReleaseAndGetAddressOf()
            )
        );

#ifdef _DEBUG
        // DXGI debug components may not be installed.
        if (FAILED(hr) && factoryFlags != 0)
        {
            ENGINE_LOG_WARN(
                "DXGI debug factory unavailable. "
                "Retrying without debug flags. "
                "HRESULT: 0x{:08X}",
                static_cast<unsigned int>(hr)
            );

            hr = CreateDXGIFactory2(
                0,
                IID_PPV_ARGS(
                    m_Factory.ReleaseAndGetAddressOf()
                )
            );
        }
#endif

        if (FAILED(hr))
        {
            ENGINE_LOG_ERROR(
                "CreateDXGIFactory2 failed: 0x{:08X}",
                static_cast<unsigned int>(hr)
            );

            return false;
        }

        ENGINE_LOG_INFO("DXGI Factory initialized");

        if (!EnumerateHardwareAdapters())
        {
            if (!allowWarp || !SelectWarpAdapter())
            {
                ENGINE_LOG_ERROR(
                    "No compatible D3D12 adapter found"
                );

                Shutdown();
                return false;
            }
        }

        ENGINE_LOG_INFO(
            "Selected GPU: {}",
            m_SelectedInfo.Name
        );

        ENGINE_LOG_INFO(
            "GPU Type: {}",
            m_SelectedInfo.IsSoftware
                ? "Software (WARP)"
                : "Hardware"
        );

        ENGINE_LOG_INFO(
            "Dedicated VRAM: {:.2f} GiB",
            static_cast<double>(
                m_SelectedInfo.DedicatedVideoMemory
            ) / (1024.0 * 1024.0 * 1024.0)
        );

        ENGINE_LOG_INFO(
            "Vendor ID: 0x{:04X}",
            m_SelectedInfo.VendorId
        );

        return true;
    }

    bool GraphicsAdapter::EnumerateHardwareAdapters()
    {
        for (UINT index = 0; ; ++index)
        {
            Microsoft::WRL::ComPtr<IDXGIAdapter4>
                adapter;

            const HRESULT hr =
                m_Factory->EnumAdapterByGpuPreference(
                    index,
                    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                    IID_PPV_ARGS(
                        adapter.GetAddressOf()
                    )
                );

            if (hr == DXGI_ERROR_NOT_FOUND)
            {
                break;
            }

            if (FAILED(hr))
            {
                ENGINE_LOG_ERROR(
                    "GPU enumeration failed: 0x{:08X}",
                    static_cast<unsigned int>(hr)
                );
                return false;
            }

            auto info = GetAdapterInfo(adapter.Get());

            info.SupportsD3D12 =
                SupportsD3D12(adapter.Get());

            ENGINE_LOG_INFO(
                "GPU [{}]: {} | {} | D3D12: {}",
                index,
                info.Name,
                info.IsSoftware
                    ? "Software"
                    : "Hardware",
                info.SupportsD3D12
                    ? "Supported"
                    : "Unsupported"
            );

            m_AvailableAdapters.push_back(info);

            if (info.IsSoftware || !info.SupportsD3D12)
            {
                continue;
            }

            // Keep the first compatible adapter.
            // DXGI already orders by performance preference.
            if (!m_SelectedAdapter)
            {
                m_SelectedAdapter = adapter;
                m_SelectedInfo = info;
            }
        }

        return m_SelectedAdapter != nullptr;
    }

    bool GraphicsAdapter::SelectWarpAdapter()
    {
        ENGINE_LOG_WARN(
            "No compatible hardware GPU. Trying WARP."
        );

        Microsoft::WRL::ComPtr<IDXGIAdapter4>
            warpAdapter;

        const HRESULT hr = m_Factory->EnumWarpAdapter(
            IID_PPV_ARGS(
                warpAdapter.GetAddressOf()
            )
        );

        if (FAILED(hr))
        {
            ENGINE_LOG_ERROR(
                "EnumWarpAdapter failed: 0x{:08X}",
                static_cast<unsigned int>(hr)
            );

            return false;
        }

        auto info = GetAdapterInfo(warpAdapter.Get());

        info.IsSoftware = true;
        info.SupportsD3D12 =
            SupportsD3D12(warpAdapter.Get());

        if (!info.SupportsD3D12)
        {
            ENGINE_LOG_ERROR(
                "WARP does not satisfy D3D12 requirements"
            );
            return false;
        }

        m_AvailableAdapters.push_back(info);
        m_SelectedAdapter = warpAdapter;
        m_SelectedInfo = info;

        return true;
    }

    bool GraphicsAdapter::SupportsD3D12(
        IDXGIAdapter4* adapter)
    {
        if (adapter == nullptr)
        {
            return false;
        }

        // Capability probe only; no device is retained.
        const HRESULT hr = D3D12CreateDevice(
            adapter,
            D3D_FEATURE_LEVEL_11_0,
            __uuidof(ID3D12Device),
            nullptr
        );

        return SUCCEEDED(hr);
    }

    GraphicsAdapterInfo GraphicsAdapter::GetAdapterInfo(
        IDXGIAdapter4* adapter)
    {
        GraphicsAdapterInfo info{};

        DXGI_ADAPTER_DESC3 description{};

        if (FAILED(adapter->GetDesc3(&description)))
        {
            ENGINE_LOG_ERROR(
                "IDXGIAdapter4::GetDesc3 failed"
            );

            return info;
        }

        info.Name = ToUtf8(description.Description);
        info.VendorId = description.VendorId;
        info.DeviceId = description.DeviceId;

        info.DedicatedVideoMemory =
            description.DedicatedVideoMemory;

        info.DedicatedSystemMemory =
            description.DedicatedSystemMemory;

        info.SharedSystemMemory =
            description.SharedSystemMemory;

        info.IsSoftware =
            (description.Flags &
                DXGI_ADAPTER_FLAG3_SOFTWARE) != 0;

        return info;
    }

    void GraphicsAdapter::Shutdown() noexcept
    {
        m_SelectedAdapter.Reset();
        m_Factory.Reset();

        m_SelectedInfo = {};
        m_AvailableAdapters.clear();
    }

    bool GraphicsAdapter::IsValid() const noexcept
    {
        return m_Factory != nullptr &&
            m_SelectedAdapter != nullptr;
    }

    IDXGIAdapter4*
    GraphicsAdapter::GetAdapter() const noexcept
    {
        return m_SelectedAdapter.Get();
    }

    IDXGIFactory6*
    GraphicsAdapter::GetFactory() const noexcept
    {
        return m_Factory.Get();
    }

    const GraphicsAdapterInfo&
    GraphicsAdapter::GetSelectedInfo() const noexcept
    {
        return m_SelectedInfo;
    }

    const std::vector<GraphicsAdapterInfo>&
    GraphicsAdapter::GetAvailableAdapters() const noexcept
    {
        return m_AvailableAdapters;
    }
}
