#include "ShaderCompiler.h"

#include "Core/Debug/HResult.h"
#include "Core/Logging/Logger.h"

#include <cstdint>
#include <vector>

namespace MyGameEngine
{
    ShaderCompiler::~ShaderCompiler()
    {
        Shutdown();
    }

    bool ShaderCompiler::Initialize()
    {
        if (IsValid())
        {
            ENGINE_LOG_WARN("ShaderCompiler already initialized");
            return false;
        }

        Shutdown();

        HRESULT hr = DxcCreateInstance(
            CLSID_DxcUtils,
            IID_PPV_ARGS(m_Utils.GetAddressOf())
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            Shutdown();
            return false;
        }

        hr = DxcCreateInstance(
            CLSID_DxcCompiler,
            IID_PPV_ARGS(m_Compiler.GetAddressOf())
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            Shutdown();
            return false;
        }

        hr = m_Utils->CreateDefaultIncludeHandler(
            m_IncludeHandler.GetAddressOf()
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            Shutdown();
            return false;
        }

        ENGINE_LOG_INFO("ShaderCompiler initialized");
        return true;
    }

    bool ShaderCompiler::Compile(
        const ShaderCompileOptions& options,
        CompiledShader& output)
    {
        output.Bytecode.Reset();

        if (!IsValid())
        {
            ENGINE_LOG_ERROR("ShaderCompiler not initialized");
            return false;
        }

        if (options.FilePath.empty() || options.EntryPoint.empty())
        {
            ENGINE_LOG_ERROR("Invalid shader compile options");
            return false;
        }

        const auto absolutePath = std::filesystem::absolute(
            options.FilePath
        ).lexically_normal();

        Microsoft::WRL::ComPtr<IDxcBlobEncoding> source;
        HRESULT hr = m_Utils->LoadFile(
            absolutePath.c_str(),
            nullptr,
            source.GetAddressOf()
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            ENGINE_LOG_ERROR(
                "Failed to load shader file: {}",
                absolutePath.string()
            );
            return false;
        }

        const DxcBuffer sourceBuffer{
            source->GetBufferPointer(),
            source->GetBufferSize(),
            DXC_CP_UTF8
        };

        const std::wstring includeDirectory =
            absolutePath.parent_path().wstring();

        std::vector<LPCWSTR> arguments;
        arguments.push_back(L"-E");
        arguments.push_back(options.EntryPoint.c_str());
        arguments.push_back(L"-T");
        arguments.push_back(GetShaderProfile(options.Stage));
        arguments.push_back(L"-HV");
        arguments.push_back(L"2021");
        arguments.push_back(L"-I");
        arguments.push_back(includeDirectory.c_str());

        if (options.EnableDebugInfo)
        {
            arguments.push_back(L"-Zi");
            arguments.push_back(L"-Qembed_debug");
        }

        arguments.push_back(
            options.EnableOptimization ? L"-O3" : L"-Od"
        );

        if (options.WarningsAsErrors)
        {
            arguments.push_back(L"-WX");
        }

        Microsoft::WRL::ComPtr<IDxcResult> result;
        hr = m_Compiler->Compile(
            &sourceBuffer,
            arguments.data(),
            static_cast<UINT32>(arguments.size()),
            m_IncludeHandler.Get(),
            IID_PPV_ARGS(result.GetAddressOf())
        );

        if (!ENGINE_CHECK_HR(hr))
        {
            ENGINE_LOG_ERROR("DXC invocation failed");
            return false;
        }

        Microsoft::WRL::ComPtr<IDxcBlobUtf8> diagnostics;
        hr = result->GetOutput(
            DXC_OUT_ERRORS,
            IID_PPV_ARGS(diagnostics.GetAddressOf()),
            nullptr
        );

        if (SUCCEEDED(hr) && diagnostics &&
            diagnostics->GetStringLength() > 0)
        {
            ENGINE_LOG_WARN(
                "DXC diagnostics:\n{}",
                diagnostics->GetStringPointer()
            );
        }

        HRESULT compileStatus = E_FAIL;

        if (!ENGINE_CHECK_HR(result->GetStatus(&compileStatus)))
        {
            return false;
        }

        if (FAILED(compileStatus))
        {
            ENGINE_LOG_ERROR(
                "Shader compilation failed: 0x{:08X}",
                static_cast<unsigned int>(compileStatus)
            );
            return false;
        }

        hr = result->GetOutput(
            DXC_OUT_OBJECT,
            IID_PPV_ARGS(output.Bytecode.GetAddressOf()),
            nullptr
        );

        if (!ENGINE_CHECK_HR(hr) || !output.IsValid())
        {
            ENGINE_LOG_ERROR("DXC returned invalid bytecode");
            output.Bytecode.Reset();
            return false;
        }

        ENGINE_LOG_INFO(
            "Shader compiled successfully: "
            "EntryPoint={}, BytecodeSize={} bytes",
            std::filesystem::path(options.EntryPoint).string(),
            output.Bytecode->GetBufferSize()
        );
        return true;
    }

    void ShaderCompiler::Shutdown() noexcept
    {
        m_IncludeHandler.Reset();
        m_Compiler.Reset();
        m_Utils.Reset();
    }

    bool ShaderCompiler::IsValid() const noexcept
    {
        return m_Utils != nullptr &&
            m_Compiler != nullptr &&
            m_IncludeHandler != nullptr;
    }

    const wchar_t* ShaderCompiler::GetShaderProfile(
        ShaderStage stage) noexcept
    {
        switch (stage)
        {
        case ShaderStage::Vertex:
            return L"vs_6_0";
        case ShaderStage::Pixel:
            return L"ps_6_0";
        default:
            return L"vs_6_0";
        }
    }
}
