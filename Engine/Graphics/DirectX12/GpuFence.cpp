#include "GpuFence.h"

#include "Core/Debug/HResult.h"
#include "Core/Logging/Logger.h"

#include <limits>

namespace MyGameEngine
{
    GpuFence::~GpuFence()
    {
        Shutdown();
    }

    bool GpuFence::Initialize(ID3D12Device* device)
    {
        if (IsValid() || device == nullptr)
        {
            ENGINE_LOG_ERROR("Invalid GpuFence initialization");
            return false;
        }

        const HRESULT hr = device->CreateFence(
            0,
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(m_Fence.GetAddressOf())
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            return false;
        }

        m_Event = CreateEventW(nullptr, FALSE, FALSE, nullptr);

        if (m_Event == nullptr)
        {
            ENGINE_LOG_ERROR(
                "CreateEventW failed: {}",
                GetLastError()
            );
            Shutdown();
            return false;
        }

        m_NextFenceValue = 1;
        ENGINE_LOG_INFO("GPU Fence initialized");
        return true;
    }

    bool GpuFence::Signal(
        ID3D12CommandQueue* queue,
        std::uint64_t& signaledValue)
    {
        signaledValue = 0;

        if (!IsValid() || queue == nullptr)
        {
            ENGINE_LOG_ERROR("Invalid GPU Signal");
            return false;
        }

        if (m_NextFenceValue ==
            (std::numeric_limits<std::uint64_t>::max)())
        {
            ENGINE_LOG_CRITICAL("GPU Fence value overflow");
            return false;
        }

        const std::uint64_t value = m_NextFenceValue;
        const HRESULT hr = queue->Signal(m_Fence.Get(), value);

        if (!ENGINE_CHECK_HR(hr))
        {
            return false;
        }

        ++m_NextFenceValue;
        signaledValue = value;

        ENGINE_LOG_DEBUG("GPU Fence signaled: {}", value);
        return true;
    }

    bool GpuFence::Wait(std::uint64_t value)
    {
        if (!IsValid())
        {
            ENGINE_LOG_ERROR("GPU Fence is invalid");
            return false;
        }

        if (value == 0)
        {
            return true;
        }

        const std::uint64_t completed = GetCompletedValue();

        if (completed == UINT64_MAX)
        {
            ENGINE_LOG_ERROR("GPU Fence is in device-removed state");
            return false;
        }

        if (completed >= value)
        {
            return true;
        }

        if (!ENGINE_CHECK_HR(
                m_Fence->SetEventOnCompletion(value, m_Event)))
        {
            return false;
        }

        ENGINE_LOG_DEBUG("Waiting for GPU Fence {}", value);

        const DWORD result = WaitForSingleObject(m_Event, INFINITE);

        if (result != WAIT_OBJECT_0)
        {
            ENGINE_LOG_ERROR(
                "GPU Fence wait failed: {}",
                GetLastError()
            );
            return false;
        }

        const auto finalValue = GetCompletedValue();

        if (finalValue == UINT64_MAX || finalValue < value)
        {
            ENGINE_LOG_ERROR("GPU Fence completion invalid");
            return false;
        }

        return true;
    }

    bool GpuFence::WaitForGpu(ID3D12CommandQueue* queue)
    {
        std::uint64_t value = 0;

        if (!Signal(queue, value))
        {
            return false;
        }

        return Wait(value);
    }

    bool GpuFence::IsComplete(std::uint64_t value) const
    {
        if (!IsValid())
        {
            return false;
        }

        if (value == 0)
        {
            return true;
        }

        const auto completed = GetCompletedValue();
        return completed != UINT64_MAX && completed >= value;
    }

    std::uint64_t GpuFence::GetCompletedValue() const
    {
        return m_Fence ? m_Fence->GetCompletedValue() : 0;
    }

    bool GpuFence::IsValid() const noexcept
    {
        return m_Fence != nullptr && m_Event != nullptr;
    }

    void GpuFence::Shutdown() noexcept
    {
        if (m_Event != nullptr)
        {
            CloseHandle(m_Event);
            m_Event = nullptr;
        }

        m_Fence.Reset();
        m_NextFenceValue = 1;
    }
}
