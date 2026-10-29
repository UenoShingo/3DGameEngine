#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include <cstddef>
#include <cstdint>

namespace VortexEngine::Platform
{
    struct WindowState final
    {
        int Width = 0;
        int Height = 0;
        unsigned int Dpi = USER_DEFAULT_SCREEN_DPI;
        bool IsMinimized = false;
        bool IsResizing = false;
    };

    enum class WindowEventType : std::uint8_t
    {
        Resized,
        DpiChanged,
        Minimized,
        Restored
    };

    struct WindowEvent final
    {
        WindowEventType Type = WindowEventType::Resized;
        WindowState State{};
    };

    class NativeWindow final
    {
    public:
        NativeWindow() = default;
        ~NativeWindow();

        NativeWindow(const NativeWindow&) = delete;
        NativeWindow& operator=(const NativeWindow&) = delete;
        NativeWindow(NativeWindow&&) = delete;
        NativeWindow& operator=(NativeWindow&&) = delete;

        [[nodiscard]]
        bool Create(
            HINSTANCE instance,
            int width,
            int height,
            const wchar_t* title
        );

        void Show(int command) const;
        void Destroy() noexcept;

        [[nodiscard]]
        HWND GetHandle() const noexcept
        {
            return m_handle;
        }

        [[nodiscard]]
        const WindowState& GetState() const noexcept
        {
            return m_state;
        }

        [[nodiscard]]
        bool PollEvent(WindowEvent& event) noexcept;

    private:
        static LRESULT CALLBACK WindowProc(
            HWND hwnd,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
        );

        LRESULT HandleMessage(
            HWND hwnd,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
        );

        void PushEvent(WindowEventType type) noexcept;
        void UpdateClientSize(HWND hwnd) noexcept;

        static constexpr std::size_t EventCapacity = 32;
        static constexpr const wchar_t* ClassName =
            L"VortexEngineNativeWindow";

        HINSTANCE m_instance = nullptr;
        HWND m_handle = nullptr;
        bool m_classRegistered = false;

        WindowState m_state{};

        WindowEvent m_events[EventCapacity]{};
        std::size_t m_eventHead = 0;
        std::size_t m_eventCount = 0;
    };
}
