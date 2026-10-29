#include <cstdlib>

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
        if (PeekMessageW(
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
                continue;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        else
        {
            // No rendering yet.
            // Sleep until a new message arrives.
            if (!WaitMessage())
            {
                exitCode = EXIT_FAILURE;
                running = false;
            }
        }
    }

    window.Destroy();

    return exitCode;
}
