#include <cstdlib>
#include <string>

#include <Windows.h>

#include "Platform/Windows/NativeWindow.h"

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PWSTR commandLine,
    int showCommand)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(commandLine);

    // Must be configured before window creation.
    // If a manifest has already configured DPI awareness,
    // this call may fail without being fatal.
    SetProcessDpiAwarenessContext(
        DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
    );

    constexpr int WindowWidth = 1280;
    constexpr int WindowHeight = 720;

    VortexEngine::Platform::NativeWindow window;

    if (!window.Create(
            hInstance,
            WindowWidth,
            WindowHeight,
            L"VortexEngine Editor"))
    {
        MessageBoxW(
            nullptr,
            L"Failed to create the application window.",
            L"VortexEngine Error",
            MB_OK | MB_ICONERROR
        );

        return EXIT_FAILURE;
    }

    window.Show(showCommand);

    MSG message{};
    bool running = true;
    int exitCode = EXIT_SUCCESS;

    while (running)
    {
        while (PeekMessageW(
            &message,
            nullptr,
            0,
            0,
            PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                exitCode =
                    static_cast<int>(message.wParam);

                running = false;
                break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        if (!running)
        {
            break;
        }

        VortexEngine::Platform::WindowEvent event{};

        while (window.PollEvent(event))
        {
            const auto& state = event.State;

            if (window.GetHandle() == nullptr)
            {
                continue;
            }

            std::wstring title =
                L"VortexEngine Editor | ";

            title += std::to_wstring(state.Width);
            title += L"x";
            title += std::to_wstring(state.Height);
            title += L" | DPI ";
            title += std::to_wstring(state.Dpi);

            if (state.IsMinimized)
            {
                title += L" | Minimized";
            }

            SetWindowTextW(
                window.GetHandle(),
                title.c_str()
            );
        }

        // Rendering will be added in later commits.
        if (!WaitMessage())
        {
            exitCode = EXIT_FAILURE;
            break;
        }
    }

    window.Destroy();

    return exitCode;
}
