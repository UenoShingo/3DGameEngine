#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace MyGameEngine
{
    struct GraphicsAdapterInfo
    {
        std::string Name;

        std::uint32_t VendorId = 0;
        std::uint32_t DeviceId = 0;

        std::uint64_t DedicatedVideoMemory = 0;
        std::uint64_t DedicatedSystemMemory = 0;
        std::uint64_t SharedSystemMemory = 0;

        bool IsSoftware = false;
        bool SupportsD3D12 = false;
    };

    class GraphicsAdapter final
    {
    public:
        GraphicsAdapter() = default;
        ~GraphicsAdapter() = default;

        GraphicsAdapter(
            const GraphicsAdapter&) = delete;

        GraphicsAdapter& operator=(
            const GraphicsAdapter&) = delete;

        [[nodiscard]]
        bool Initialize(bool allowWarp = true);

        void Shutdown() noexcept;

        [[nodiscard]]
        bool IsValid() const noexcept;

        [[nodiscard]]
        IDXGIAdapter4* GetAdapter() const noexcept;

        [[nodiscard]]
        IDXGIFactory6* GetFactory() const noexcept;

        [[nodiscard]]
        const GraphicsAdapterInfo&
        GetSelectedInfo() const noexcept;

        [[nodiscard]]
        const std::vector<GraphicsAdapterInfo>&
        GetAvailableAdapters() const noexcept;

    private:
        [[nodiscard]]
        bool EnumerateHardwareAdapters();

        [[nodiscard]]
        bool SelectWarpAdapter();

        [[nodiscard]]
        static bool SupportsD3D12(
            IDXGIAdapter4* adapter);

        [[nodiscard]]
        static GraphicsAdapterInfo GetAdapterInfo(
            IDXGIAdapter4* adapter);

        Microsoft::WRL::ComPtr<IDXGIFactory6>
            m_Factory;

        Microsoft::WRL::ComPtr<IDXGIAdapter4>
            m_SelectedAdapter;

        GraphicsAdapterInfo m_SelectedInfo;

        std::vector<GraphicsAdapterInfo>
            m_AvailableAdapters;
    };
}
