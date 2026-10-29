#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

namespace VortexEngine::Platform
{
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

    private:
        static LRESULT CALLBACK WindowProc(
            HWND hwnd,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
        );

        HINSTANCE m_instance = nullptr;
        HWND m_handle = nullptr;
        bool m_classRegistered = false;

        static constexpr const wchar_t* ClassName =
            L"VortexEngineNativeWindow";
    };
}
