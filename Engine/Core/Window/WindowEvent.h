#pragma once

#include <cstdint>

namespace MyGameEngine
{
    enum class WindowEventType
    {
        Closed,
        Resized,
        FocusGained,
        FocusLost,
        KeyDown,
        KeyUp
    };

    struct WindowEvent
    {
        WindowEventType Type;

        // Resize event data
        std::uint32_t Width = 0;
        std::uint32_t Height = 0;

        // Keyboard event data
        std::uint32_t KeyCode = 0;
        bool IsRepeat = false;
    };
}
