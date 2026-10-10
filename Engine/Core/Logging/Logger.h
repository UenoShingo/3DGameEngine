#pragma once

#include <spdlog/spdlog.h>

namespace MyGameEngine
{
    class Logger final
    {
    public:
        Logger() = delete;

        [[nodiscard]]
        static bool Initialize() noexcept;

        static void Shutdown() noexcept;
    };
}

// Logging macros
#define ENGINE_LOG_TRACE(...) \
    SPDLOG_TRACE(__VA_ARGS__)

#define ENGINE_LOG_DEBUG(...) \
    SPDLOG_DEBUG(__VA_ARGS__)

#define ENGINE_LOG_INFO(...) \
    SPDLOG_INFO(__VA_ARGS__)

#define ENGINE_LOG_WARN(...) \
    SPDLOG_WARN(__VA_ARGS__)

#define ENGINE_LOG_ERROR(...) \
    SPDLOG_ERROR(__VA_ARGS__)

#define ENGINE_LOG_CRITICAL(...) \
    SPDLOG_CRITICAL(__VA_ARGS__)
