#include "GraphicsContext.h"

#include "Core/Debug/HResult.h"
#include "Core/Logging/Logger.h"
#include "Graphics/DirectX12/D3D12Device.h"

namespace MyGameEngine
{
    GraphicsContext::~GraphicsContext()
    {
        if (!Shutdown())
        {
            ENGINE_LOG_CRITICAL(
                "GraphicsContext destroyed without "
                "confirmed GPU completion"
            );
        }
    }

    bool GraphicsContext::Initialize(const D3D12Device& device)
    {
        if (m_State != CommandContextState::Uninitialized ||
            !device.IsValid())
        {
            ENGINE_LOG_ERROR("Invalid GraphicsContext initialization");
            return false;
        }

        m_Device = device.GetDevice();

        D3D12_COMMAND_QUEUE_DESC queueDesc{};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        HRESULT hr = m_Device->CreateCommandQueue(
            &queueDesc,
            IID_PPV_ARGS(m_CommandQueue.GetAddressOf())
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            m_State = CommandContextState::Failed;
            return false;
        }

        for (auto& frame : m_Frames)
        {
            hr = m_Device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(frame.CommandAllocator.GetAddressOf())
            );

            if (!ENGINE_CHECK_HR(hr))
            {
                m_State = CommandContextState::Failed;
                return false;
            }
        }

        hr = m_Device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_Frames[0].CommandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(m_CommandList.GetAddressOf())
        );

        if (!ENGINE_CHECK_HR(hr) ||
            !ENGINE_CHECK_HR(m_CommandList->Close()))
        {
            m_State = CommandContextState::Failed;
            return false;
        }

        if (!m_Fence.Initialize(m_Device.Get()))
        {
            m_State = CommandContextState::Failed;
            return false;
        }

        m_CurrentFrameIndex = 0;
        m_HasUntrackedSubmission = false;
        m_State = CommandContextState::Ready;

        ENGINE_LOG_INFO(
            "GraphicsContext initialized with {} frames",
            FrameCount
        );
        return true;
    }

    bool GraphicsContext::BeginFrame(std::uint32_t frameIndex)
    {
        if (m_State != CommandContextState::Ready ||
            frameIndex >= FrameCount)
        {
            ENGINE_LOG_ERROR(
                "BeginFrame rejected: invalid state or index"
            );
            return false;
        }

        FrameResource& frame = m_Frames[frameIndex];

        if (!m_Fence.Wait(frame.FenceValue) ||
            !ENGINE_CHECK_HR(frame.CommandAllocator->Reset()) ||
            !ENGINE_CHECK_HR(
                m_CommandList->Reset(
                    frame.CommandAllocator.Get(),
                    nullptr)))
        {
            m_State = CommandContextState::Failed;
            return false;
        }

        m_CurrentFrameIndex = frameIndex;
        m_State = CommandContextState::Recording;

        ENGINE_LOG_DEBUG("BeginFrame: {}", frameIndex);
        return true;
    }

    bool GraphicsContext::EndFrame()
    {
        if (m_State != CommandContextState::Recording)
        {
            ENGINE_LOG_ERROR("EndFrame rejected: not recording");
            return false;
        }

        if (!ENGINE_CHECK_HR(m_CommandList->Close()))
        {
            m_State = CommandContextState::Failed;
            return false;
        }

        m_State = CommandContextState::Closed;

        ID3D12CommandList* lists[] = { m_CommandList.Get() };
        m_CommandQueue->ExecuteCommandLists(1, lists);
        m_HasUntrackedSubmission = true;

        std::uint64_t fenceValue = 0;

        if (!m_Fence.Signal(m_CommandQueue.Get(), fenceValue))
        {
            m_State = CommandContextState::Failed;
            return false;
        }

        m_Frames[m_CurrentFrameIndex].FenceValue = fenceValue;
        m_HasUntrackedSubmission = false;
        m_State = CommandContextState::Ready;

        ENGINE_LOG_DEBUG(
            "EndFrame: {} | Fence: {}",
            m_CurrentFrameIndex,
            fenceValue
        );
        return true;
    }

    bool GraphicsContext::WaitForGpu()
    {
        if (!m_CommandQueue || !m_Fence.IsValid())
        {
            ENGINE_LOG_ERROR("WaitForGpu: context not initialized");
            return false;
        }

        if (m_State == CommandContextState::Recording)
        {
            ENGINE_LOG_ERROR("WaitForGpu called during recording");
            return false;
        }

        if (!m_Fence.WaitForGpu(m_CommandQueue.Get()))
        {
            ENGINE_LOG_ERROR("GPU synchronization failed");
            m_State = CommandContextState::Failed;
            return false;
        }

        m_HasUntrackedSubmission = false;
        ENGINE_LOG_DEBUG("GPU queue is idle");
        return true;
    }

    bool GraphicsContext::PrepareForResize()
    {
        if (m_State != CommandContextState::Ready)
        {
            ENGINE_LOG_ERROR("Resize requires an idle recording state");
            return false;
        }

        if (!WaitForGpu())
        {
            return false;
        }

        for (auto& frame : m_Frames)
        {
            frame.FenceValue = 0;
        }

        ENGINE_LOG_INFO("GPU synchronized for SwapChain resize");
        return true;
    }

    bool GraphicsContext::Shutdown()
    {
        if (m_State == CommandContextState::Uninitialized)
        {
            return true;
        }

        if (m_State == CommandContextState::Recording && m_CommandList)
        {
            const HRESULT hr = m_CommandList->Close();

            if (FAILED(hr))
            {
                ENGINE_LOG_ERROR(
                    "CommandList close failed: 0x{:08X}",
                    static_cast<unsigned int>(hr)
                );
            }
            else
            {
                m_State = CommandContextState::Closed;
            }
        }

        if (m_CommandQueue && m_Fence.IsValid() && !WaitForGpu())
        {
            ENGINE_LOG_CRITICAL("GPU work could not be synchronized");
            m_State = CommandContextState::Failed;
            return false;
        }

        m_CommandList.Reset();

        for (auto& frame : m_Frames)
        {
            frame.CommandAllocator.Reset();
            frame.FenceValue = 0;
        }

        m_Fence.Shutdown();
        m_CommandQueue.Reset();
        m_Device.Reset();

        m_CurrentFrameIndex = 0;
        m_HasUntrackedSubmission = false;
        m_State = CommandContextState::Uninitialized;

        ENGINE_LOG_INFO("GraphicsContext shutdown");
        return true;
    }

    bool GraphicsContext::IsValid() const noexcept
    {
        if (!m_Device || !m_CommandQueue ||
            !m_CommandList || !m_Fence.IsValid())
        {
            return false;
        }

        for (const auto& frame : m_Frames)
        {
            if (!frame.CommandAllocator)
            {
                return false;
            }
        }

        return m_State != CommandContextState::Uninitialized &&
            m_State != CommandContextState::Failed;
    }

    CommandContextState GraphicsContext::GetState() const noexcept
    {
        return m_State;
    }

    ID3D12CommandQueue* GraphicsContext::GetCommandQueue() const noexcept
    {
        return m_CommandQueue.Get();
    }

    ID3D12GraphicsCommandList* GraphicsContext::GetCommandList() const noexcept
    {
        return m_CommandList.Get();
    }

    std::uint32_t GraphicsContext::GetCurrentFrameIndex() const noexcept
    {
        return m_CurrentFrameIndex;
    }

    std::uint64_t GraphicsContext::GetFrameFenceValue(
        std::uint32_t index) const noexcept
    {
        return index < FrameCount ? m_Frames[index].FenceValue : 0;
    }
}
