#include "D3D12Device.h"

#include "Core/Debug/HResult.h"
#include "Core/Logging/Logger.h"
#include "Graphics/DirectX12/GraphicsAdapter.h"

#include <iterator>

namespace MyGameEngine
{
    D3D12Device::~D3D12Device()
    {
        Shutdown();
    }

    bool D3D12Device::ConfigureDebugLayer(
        const D3D12DeviceOptions& options)
    {
        if (!options.EnableDebugLayer)
        {
            ENGINE_LOG_INFO(
                "D3D12 Debug Layer disabled"
            );
            return true;
        }

        Microsoft::WRL::ComPtr<ID3D12Debug>
            debugController;

        HRESULT hr = D3D12GetDebugInterface(
            IID_PPV_ARGS(&debugController)
        );

        if (FAILED(hr))
        {
            ENGINE_LOG_WARN(
                "D3D12 Debug Layer unavailable: "
                "0x{:08X}",
                static_cast<unsigned int>(hr)
            );

            // Debug components are optional.
            // Continue without validation.
            return true;
        }

        debugController->EnableDebugLayer();

        ENGINE_LOG_INFO(
            "D3D12 Debug Layer enabled"
        );

        if (options.EnableGpuValidation)
        {
            Microsoft::WRL::ComPtr<ID3D12Debug1>
                debugController1;

            hr = debugController.As(&debugController1);

            if (SUCCEEDED(hr))
            {
                debugController1->
                    SetEnableGPUBasedValidation(TRUE);

                ENGINE_LOG_INFO(
                    "GPU-Based Validation enabled"
                );
            }
            else
            {
                ENGINE_LOG_WARN(
                    "GPU-Based Validation unavailable: "
                    "0x{:08X}",
                    static_cast<unsigned int>(hr)
                );
            }
        }

        return true;
    }

    bool D3D12Device::Initialize(
        const GraphicsAdapter& adapter)
    {
        Shutdown();

        if (!adapter.IsValid())
        {
            ENGINE_LOG_ERROR(
                "Cannot create D3D12 Device: "
                "invalid graphics adapter"
            );

            return false;
        }

        ENGINE_LOG_INFO(
            "Creating D3D12 Device"
        );

        const HRESULT hr = D3D12CreateDevice(
            adapter.GetAdapter(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(
                m_Device.GetAddressOf()
            )
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            Shutdown();
            return false;
        }

        ENGINE_LOG_INFO(
            "D3D12 Device created successfully"
        );

        ConfigureInfoQueue();

        if (!QueryCapabilities())
        {
            ENGINE_LOG_ERROR(
                "Failed to query D3D12 capabilities"
            );

            Shutdown();
            return false;
        }

        return true;
    }

    bool D3D12Device::QueryCapabilities()
    {
        // Query common Direct3D 12 capabilities.
        D3D12_FEATURE_DATA_D3D12_OPTIONS options{};

        HRESULT hr = m_Device->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS,
            &options,
            sizeof(options)
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            return false;
        }

        m_Capabilities.ResourceBindingTier =
            options.ResourceBindingTier;

        m_Capabilities.TiledResourcesTier =
            options.TiledResourcesTier;

        m_Capabilities.ResourceHeapTier2 =
            options.ResourceHeapTier ==
            D3D12_RESOURCE_HEAP_TIER_2;

        constexpr D3D_FEATURE_LEVEL levels[] =
        {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };

        D3D12_FEATURE_DATA_FEATURE_LEVELS levelData{};
        levelData.NumFeatureLevels =
            static_cast<UINT>(std::size(levels));

        levelData.pFeatureLevelsRequested = levels;

        hr = m_Device->CheckFeatureSupport(
            D3D12_FEATURE_FEATURE_LEVELS,
            &levelData,
            sizeof(levelData)
        );

        if (FAILED(hr))
        {
            // Older drivers may reject newer queried levels.
            constexpr D3D_FEATURE_LEVEL fallbackLevels[] =
            {
                D3D_FEATURE_LEVEL_12_1,
                D3D_FEATURE_LEVEL_12_0,
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0
            };

            levelData.NumFeatureLevels =
                static_cast<UINT>(
                    std::size(fallbackLevels)
                );

            levelData.pFeatureLevelsRequested =
                fallbackLevels;

            hr = m_Device->CheckFeatureSupport(
                D3D12_FEATURE_FEATURE_LEVELS,
                &levelData,
                sizeof(levelData)
            );
        }

        if (!ENGINE_CHECK_HR(hr))
        {
            return false;
        }

        m_Capabilities.MaxFeatureLevel =
            levelData.MaxSupportedFeatureLevel;

        ENGINE_LOG_INFO(
            "D3D12 Feature Level: 0x{:04X}",
            static_cast<unsigned int>(
                m_Capabilities.MaxFeatureLevel
            )
        );

        ENGINE_LOG_INFO(
            "Resource Binding Tier: {}",
            static_cast<int>(
                m_Capabilities.ResourceBindingTier
            )
        );

        ENGINE_LOG_INFO(
            "Tiled Resources Tier: {}",
            static_cast<int>(
                m_Capabilities.TiledResourcesTier
            )
        );

        ENGINE_LOG_INFO(
            "Resource Heap Tier 2: {}",
            m_Capabilities.ResourceHeapTier2
                ? "Supported"
                : "Unsupported"
        );

        return true;
    }

    void D3D12Device::ConfigureInfoQueue()
    {
        const HRESULT hr =
            m_Device.As(&m_InfoQueue);

        if (FAILED(hr))
        {
            ENGINE_LOG_INFO(
                "D3D12 InfoQueue unavailable"
            );
            return;
        }

        // Break into Visual Studio debugger
        // when severe validation errors occur.
        m_InfoQueue->SetBreakOnSeverity(
            D3D12_MESSAGE_SEVERITY_CORRUPTION,
            TRUE
        );

        m_InfoQueue->SetBreakOnSeverity(
            D3D12_MESSAGE_SEVERITY_ERROR,
            TRUE
        );

        ENGINE_LOG_INFO(
            "D3D12 InfoQueue configured"
        );
    }

    void D3D12Device::Shutdown() noexcept
    {
        m_InfoQueue.Reset();
        m_Device.Reset();

        m_Capabilities = {};
    }

    bool D3D12Device::IsValid() const noexcept
    {
        return m_Device != nullptr;
    }

    ID3D12Device*
    D3D12Device::GetDevice() const noexcept
    {
        return m_Device.Get();
    }

    const D3D12DeviceCapabilities&
    D3D12Device::GetCapabilities() const noexcept
    {
        return m_Capabilities;
    }
}
