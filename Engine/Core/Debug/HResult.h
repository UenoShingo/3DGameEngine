#pragma once

#include <Windows.h>

#include "Core/Logging/Logger.h"

namespace MyGameEngine
{
    [[nodiscard]]
    inline bool CheckHResult(
        HRESULT result,
        const char* expression,
        const char* file,
        int line)
    {
        if (SUCCEEDED(result))
        {
            return true;
        }

        ENGINE_LOG_ERROR(
            "HRESULT failed: {} "
            "(0x{:08X}) at {}:{}",
            expression,
            static_cast<unsigned int>(result),
            file,
            line
        );

        return false;
    }
}

#define ENGINE_CHECK_HR(expression) \
    ::MyGameEngine::CheckHResult( \
        (expression), \
        #expression, \
        __FILE__, \
        __LINE__ \
    )
