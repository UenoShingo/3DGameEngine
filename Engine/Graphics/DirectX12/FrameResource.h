#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

namespace MyGameEngine
{
    struct FrameResource final
    {
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator>
            CommandAllocator;

        // 0 means that no GPU submission currently owns
        // this allocator.
        std::uint64_t FenceValue = 0;
    };
}
