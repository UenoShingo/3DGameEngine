#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include <cstdint>
#include <string>

namespace MyGameEngine
{
    struct WindowProperties
    {
        std::wstring Title = L"MyGameEngine";
        std::uint32_t Width = 1280;
        std::uint32_t Height = 720;
    };

    class Window final
    {
    public:
        Window() = default;
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

        [[nodiscard]]
        bool Create(const WindowProperties& properties);

        void Show() const;
        void Destroy() noexcept;

        [[nodiscard]]
        HWND GetHandle() const noexcept;

        [[nodiscard]]
        bool IsValid() const noexcept;

        [[nodiscard]]
        std::uint32_t GetWidth() const noexcept;

        [[nodiscard]]
        std::uint32_t GetHeight() const noexcept;

    private:
        static LRESULT CALLBACK WindowProc(
            HWND hwnd,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
        );

        HWND m_Handle = nullptr;
        HINSTANCE m_Instance = nullptr;

        std::uint32_t m_Width = 0;
        std::uint32_t m_Height = 0;

        bool m_ClassRegistered = false;

        static constexpr const wchar_t* ClassName =
            L"MyGameEngineWindowClass";
    };
}
