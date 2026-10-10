#include "GraphicsContext.h"

#include "Core/Debug/HResult.h"
#include "Core/Logging/Logger.h"
#include "Graphics/DirectX12/D3D12Device.h"

#include <Windows.h>

namespace MyGameEngine
{
    GraphicsContext::~GraphicsContext()
    {
        Shutdown();
    }

    bool GraphicsContext::Initialize(
        const D3D12Device& device)
    {
        if (m_State != CommandContextState::Uninitialized)
        {
            ENGINE_LOG_ERROR(
                "GraphicsContext already initialized"
            );
            return false;
        }

        if (!device.IsValid())
        {
            ENGINE_LOG_ERROR(
                "Invalid D3D12Device"
            );
            return false;
        }

        ID3D12Device* nativeDevice = device.GetDevice();

        // Create a direct command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc{};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;

        HRESULT hr = nativeDevice->CreateCommandQueue(
            &queueDesc,
            IID_PPV_ARGS(
                m_CommandQueue.GetAddressOf()
            )
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            Shutdown();
            return false;
        }

        ENGINE_LOG_INFO("Graphics Command Queue created");

        // Create a command allocator.
        hr = nativeDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(
                m_CommandAllocator.GetAddressOf()
            )
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            Shutdown();
            return false;
        }

        ENGINE_LOG_INFO("Graphics Command Allocator created");

        // Create a graphics command list.
        hr = nativeDevice->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_CommandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(
                m_CommandList.GetAddressOf()
            )
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            Shutdown();
            return false;
        }

        // A newly created command list is recording.
        // Close it before the first Reset.
        hr = m_CommandList->Close();

        if (!ENGINE_CHECK_HR(hr))
        {
            Shutdown();
            return false;
        }

        m_State = CommandContextState::Ready;

        ENGINE_LOG_INFO(
            "GraphicsContext initialized successfully"
        );

        return true;
    }

    bool GraphicsContext::Begin()
    {
        if (m_State != CommandContextState::Ready)
        {
            ENGINE_LOG_ERROR(
                "Begin rejected: invalid context state"
            );
            return false;
        }

        // The allocator has never been submitted.
        // It is safe to use it for this first recording.
        const HRESULT hr = m_CommandList->Reset(
            m_CommandAllocator.Get(),
            nullptr
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            m_State = CommandContextState::Failed;
            return false;
        }

        m_State = CommandContextState::Recording;

        ENGINE_LOG_DEBUG(
            "Command List recording started"
        );

        return true;
    }

    bool GraphicsContext::Close()
    {
        if (m_State != CommandContextState::Recording)
        {
            ENGINE_LOG_ERROR(
                "Close rejected: command list not recording"
            );
            return false;
        }

        const HRESULT hr = m_CommandList->Close();

        if (!ENGINE_CHECK_HR(hr))
        {
            m_State = CommandContextState::Failed;
            return false;
        }

        m_State = CommandContextState::Closed;

        ENGINE_LOG_DEBUG(
            "Command List recording completed"
        );

        return true;
    }

    bool GraphicsContext::Execute()
    {
        if (m_State != CommandContextState::Closed)
        {
            ENGINE_LOG_ERROR(
                "Execute rejected: command list not closed"
            );
            return false;
        }

        ID3D12CommandList* commandLists[] =
        {
            m_CommandList.Get()
        };

        // ExecuteCommandLists returns void.
        // Validation errors are reported by the debug layer.
        m_CommandQueue->ExecuteCommandLists(
            1,
            commandLists
        );

        m_State = CommandContextState::Submitted;

        ENGINE_LOG_INFO(
            "Graphics Command List submitted to GPU"
        );

        return true;
    }

    void GraphicsContext::Shutdown() noexcept
    {
        if (m_State == CommandContextState::Recording &&
            m_CommandList)
        {
            // Best-effort closure before resource release.
            const HRESULT hr = m_CommandList->Close();

            if (FAILED(hr))
            {
                ENGINE_LOG_ERROR(
                    "Command List close during shutdown "
                    "failed: 0x{:08X}",
                    static_cast<unsigned int>(hr)
                );
            }
        }

        if (m_State == CommandContextState::Submitted &&
            m_CommandQueue &&
            m_CommandList)
        {
            Microsoft::WRL::ComPtr<ID3D12Device> device;
            HRESULT hr = m_CommandList->GetDevice(
                IID_PPV_ARGS(device.GetAddressOf())
            );

            Microsoft::WRL::ComPtr<ID3D12Fence> shutdownFence;

            if (SUCCEEDED(hr))
            {
                hr = device->CreateFence(
                    0,
                    D3D12_FENCE_FLAG_NONE,
                    IID_PPV_ARGS(shutdownFence.GetAddressOf())
                );
            }

            constexpr UINT64 shutdownFenceValue = 1;

            if (SUCCEEDED(hr))
            {
                hr = m_CommandQueue->Signal(
                    shutdownFence.Get(),
                    shutdownFenceValue
                );
            }

            HANDLE completionEvent = nullptr;

            if (SUCCEEDED(hr))
            {
                completionEvent = CreateEventW(
                    nullptr,
                    FALSE,
                    FALSE,
                    nullptr
                );

                if (completionEvent == nullptr)
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }

            if (SUCCEEDED(hr) &&
                shutdownFence->GetCompletedValue() <
                    shutdownFenceValue)
            {
                hr = shutdownFence->SetEventOnCompletion(
                    shutdownFenceValue,
                    completionEvent
                );

                if (SUCCEEDED(hr))
                {
                    const DWORD waitResult = WaitForSingleObject(
                        completionEvent,
                        INFINITE
                    );

                    if (waitResult != WAIT_OBJECT_0)
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
            }

            if (completionEvent != nullptr)
            {
                CloseHandle(completionEvent);
            }

            if (FAILED(hr))
            {
                ENGINE_LOG_ERROR(
                    "GPU completion wait during shutdown "
                    "failed: 0x{:08X}",
                    static_cast<unsigned int>(hr)
                );
            }
        }

        m_CommandList.Reset();
        m_CommandAllocator.Reset();
        m_CommandQueue.Reset();

        m_State = CommandContextState::Uninitialized;
    }

    bool GraphicsContext::IsValid() const noexcept
    {
        return m_CommandQueue != nullptr &&
            m_CommandAllocator != nullptr &&
            m_CommandList != nullptr &&
            m_State != CommandContextState::Uninitialized &&
            m_State != CommandContextState::Failed;
    }

    CommandContextState
    GraphicsContext::GetState() const noexcept
    {
        return m_State;
    }

    ID3D12CommandQueue*
    GraphicsContext::GetCommandQueue() const noexcept
    {
        return m_CommandQueue.Get();
    }

    ID3D12CommandAllocator*
    GraphicsContext::GetCommandAllocator() const noexcept
    {
        return m_CommandAllocator.Get();
    }

    ID3D12GraphicsCommandList*
    GraphicsContext::GetCommandList() const noexcept
    {
        return m_CommandList.Get();
    }
}
