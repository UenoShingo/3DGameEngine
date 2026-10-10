#pragma once

#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <wrl/client.h>

#include <cstdint>

namespace MyGameEngine
{
    class GraphicsAdapter;

    struct D3D12DeviceOptions
    {
        bool EnableDebugLayer = true;
        bool EnableGpuValidation = false;
    };

    struct D3D12DeviceCapabilities
    {
        D3D_FEATURE_LEVEL MaxFeatureLevel =
            D3D_FEATURE_LEVEL_11_0;

        D3D12_RESOURCE_BINDING_TIER ResourceBindingTier =
            D3D12_RESOURCE_BINDING_TIER_1;

        D3D12_TILED_RESOURCES_TIER TiledResourcesTier =
            D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED;

        bool ResourceHeapTier2 = false;
    };

    class D3D12Device final
    {
    public:
        D3D12Device() = default;
        ~D3D12Device();

        D3D12Device(const D3D12Device&) = delete;
        D3D12Device& operator=(const D3D12Device&) = delete;

        D3D12Device(D3D12Device&&) = delete;
        D3D12Device& operator=(D3D12Device&&) = delete;

        // Call once before any D3D12CreateDevice call.
        static bool ConfigureDebugLayer(
            const D3D12DeviceOptions& options);

        [[nodiscard]]
        bool Initialize(const GraphicsAdapter& adapter);

        void Shutdown() noexcept;

        [[nodiscard]]
        bool IsValid() const noexcept;

        [[nodiscard]]
        ID3D12Device* GetDevice() const noexcept;

        [[nodiscard]]
        const D3D12DeviceCapabilities&
        GetCapabilities() const noexcept;

    private:
        bool QueryCapabilities();
        void ConfigureInfoQueue();

        Microsoft::WRL::ComPtr<ID3D12Device>
            m_Device;

        Microsoft::WRL::ComPtr<ID3D12InfoQueue>
            m_InfoQueue;

        D3D12DeviceCapabilities m_Capabilities{};
    };
}
