#pragma once

#include <Windows.h>
#include <objidl.h>
#include <oaidl.h>
#include <dxcapi.h>
#include <wrl/client.h>

#include <filesystem>
#include <string>

#include "Graphics/Shader/CompiledShader.h"

namespace MyGameEngine
{
    enum class ShaderStage
    {
        Vertex,
        Pixel
    };

    struct ShaderCompileOptions
    {
        std::filesystem::path FilePath;
        std::wstring EntryPoint = L"VSMain";
        ShaderStage Stage = ShaderStage::Vertex;
        bool EnableDebugInfo = false;
        bool EnableOptimization = true;
        bool WarningsAsErrors = false;
    };

    class ShaderCompiler final
    {
    public:
        ShaderCompiler() = default;
        ~ShaderCompiler();

        ShaderCompiler(const ShaderCompiler&) = delete;
        ShaderCompiler& operator=(const ShaderCompiler&) = delete;

        [[nodiscard]] bool Initialize();

        [[nodiscard]]
        bool Compile(
            const ShaderCompileOptions& options,
            CompiledShader& output
        );

        void Shutdown() noexcept;

        [[nodiscard]] bool IsValid() const noexcept;

    private:
        [[nodiscard]]
        static const wchar_t* GetShaderProfile(
            ShaderStage stage
        ) noexcept;

        Microsoft::WRL::ComPtr<IDxcUtils> m_Utils;
        Microsoft::WRL::ComPtr<IDxcCompiler3> m_Compiler;
        Microsoft::WRL::ComPtr<IDxcIncludeHandler> m_IncludeHandler;
    };
}
