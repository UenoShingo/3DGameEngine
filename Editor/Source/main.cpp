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

    // Temporary modal test.
    // Replaced by the message loop in Commit 006.
    MessageBoxW(
        window.GetHandle(),
        L"Win32 window created successfully.\n"
        L"Press OK to close the application.",
        L"VortexEngine - Commit 005",
        MB_OK | MB_ICONINFORMATION
    );

    window.Destroy();

    return EXIT_SUCCESS;
}
