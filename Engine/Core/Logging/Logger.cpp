#include "Logger.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace MyGameEngine
{
    bool Logger::Initialize() noexcept
    {
        try
        {
            namespace fs = std::filesystem;

            fs::create_directories("Logs");

            auto consoleSink =
                std::make_shared<
                    spdlog::sinks::stdout_color_sink_mt>();

            auto fileSink =
                std::make_shared<
                    spdlog::sinks::rotating_file_sink_mt>(
                        "Logs/Engine.log",
                        5 * 1024 * 1024,
                        3
                    );

            std::vector<spdlog::sink_ptr> sinks
            {
                consoleSink,
                fileSink
            };

            auto logger = std::make_shared<spdlog::logger>(
                "Engine",
                sinks.begin(),
                sinks.end()
            );

            logger->set_pattern(
                "[%Y-%m-%d %H:%M:%S.%e] "
                "[%n] [%^%l%$] [thread %t] %v"
            );

#ifdef NDEBUG
            logger->set_level(spdlog::level::info);
#else
            logger->set_level(spdlog::level::debug);
#endif

            logger->flush_on(spdlog::level::warn);

            spdlog::set_default_logger(logger);

            ENGINE_LOG_INFO("Logger initialized");

            return true;
        }
        catch (const std::exception& exception)
        {
            std::cerr
                << "Logger initialization failed: "
                << exception.what() << '\n';

            return false;
        }
        catch (...)
        {
            std::cerr
                << "Unknown logger initialization error\n";

            return false;
        }
    }

    void Logger::Shutdown() noexcept
    {
        try
        {
            ENGINE_LOG_INFO("Logger shutdown");

            spdlog::shutdown();
        }
        catch (...)
        {
            std::cerr
                << "Logger shutdown failed\n";
        }
    }
}
