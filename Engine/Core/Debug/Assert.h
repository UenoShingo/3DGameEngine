#pragma once

#include <cstdlib>

#include "Core/Logging/Logger.h"

#ifndef NDEBUG

#define ENGINE_ASSERT(condition, ...)                       \
    do                                                      \
    {                                                       \
        if (!(condition))                                   \
        {                                                   \
            ENGINE_LOG_CRITICAL(                            \
                "Assertion failed: {} ({}:{})",             \
                #condition, __FILE__, __LINE__);             \
            ENGINE_LOG_CRITICAL(__VA_ARGS__);               \
            spdlog::default_logger()->flush();              \
            std::abort();                                   \
        }                                                   \
    } while (false)

#else

#define ENGINE_ASSERT(condition, ...) \
    do {} while (false)

#endif
