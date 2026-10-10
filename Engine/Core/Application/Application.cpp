#include "Application.h"

#include "Core/Logging/Logger.h"

#include <cstdlib>
#include <exception>
#include <iostream>

namespace MyGameEngine
{
    int Application::Run()
    {
        if (m_HasRun)
        {
            std::cerr
                << "Application::Run() already called\n";

            return EXIT_FAILURE;
        }

        m_HasRun = true;

        if (!Logger::Initialize())
        {
            Logger::Shutdown();
            return EXIT_FAILURE;
        }

        int exitCode = EXIT_SUCCESS;

        try
        {
            ENGINE_LOG_INFO("Application starting");

            if (!OnInitialize())
            {
                ENGINE_LOG_ERROR(
                    "Application initialization failed"
                );

                exitCode = EXIT_FAILURE;
            }
            else
            {
                m_IsRunning = true;

                ENGINE_LOG_INFO(
                    "Application initialized successfully"
                );

                while (m_IsRunning)
                {
                    OnUpdate();
                }
            }
        }
        catch (const std::exception& exception)
        {
            ENGINE_LOG_CRITICAL(
                "Unhandled exception: {}",
                exception.what()
            );

            exitCode = EXIT_FAILURE;
        }
        catch (...)
        {
            ENGINE_LOG_CRITICAL(
                "Unknown unhandled exception"
            );

            exitCode = EXIT_FAILURE;
        }

        m_IsRunning = false;

        // OnShutdown must support partial initialization.
        try
        {
            OnShutdown();
        }
        catch (const std::exception& exception)
        {
            ENGINE_LOG_CRITICAL(
                "Shutdown exception: {}",
                exception.what()
            );

            exitCode = EXIT_FAILURE;
        }
        catch (...)
        {
            ENGINE_LOG_CRITICAL(
                "Unknown shutdown exception"
            );

            exitCode = EXIT_FAILURE;
        }

        ENGINE_LOG_INFO(
            "Application stopped with exit code {}",
            exitCode
        );

        Logger::Shutdown();

        return exitCode;
    }

    void Application::RequestExit() noexcept
    {
        m_IsRunning = false;
    }

    bool Application::IsRunning() const noexcept
    {
        return m_IsRunning;
    }
}
