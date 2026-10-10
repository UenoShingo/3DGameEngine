#pragma once

#include <Windows.h>
#include <objidl.h>
#include <oaidl.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl/client.h>

namespace MyGameEngine
{
    struct CompiledShader final
    {
        Microsoft::WRL::ComPtr<IDxcBlob> Bytecode;

        [[nodiscard]]
        bool IsValid() const noexcept
        {
            return Bytecode != nullptr &&
                Bytecode->GetBufferSize() > 0;
        }

        [[nodiscard]]
        D3D12_SHADER_BYTECODE GetD3D12Bytecode()
            const noexcept
        {
            D3D12_SHADER_BYTECODE result{};

            if (IsValid())
            {
                result.pShaderBytecode =
                    Bytecode->GetBufferPointer();
                result.BytecodeLength =
                    Bytecode->GetBufferSize();
            }

            return result;
        }
    };
}
