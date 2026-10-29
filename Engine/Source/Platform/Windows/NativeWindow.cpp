#include "Platform/Windows/NativeWindow.h"

namespace VortexEngine::Platform
{
    NativeWindow::~NativeWindow()
    {
        Destroy();
    }

    bool NativeWindow::Create(
        HINSTANCE instance,
        int width,
        int height,
        const wchar_t* title)
    {
        if (m_handle != nullptr ||
            m_classRegistered ||
            instance == nullptr ||
            title == nullptr ||
            width <= 0 ||
            height <= 0)
        {
            return false;
        }

        m_instance = instance;
        m_state = {};
        m_eventHead = 0;
        m_eventCount = 0;

        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = &NativeWindow::WindowProc;
        windowClass.hInstance = instance;
        windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        windowClass.hbrBackground =
            reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        windowClass.lpszClassName = ClassName;

        if (RegisterClassExW(&windowClass) == 0)
        {
            m_instance = nullptr;
            return false;
        }

        m_classRegistered = true;

        const DWORD style = WS_OVERLAPPEDWINDOW;
        const DWORD exStyle = 0;
        const UINT dpi = GetDpiForSystem();

        RECT rect{ 0, 0, width, height };

        if (!AdjustWindowRectExForDpi(
                &rect,
                style,
                FALSE,
                exStyle,
                dpi))
        {
            Destroy();
            return false;
        }

        const int outerWidth = rect.right - rect.left;
        const int outerHeight = rect.bottom - rect.top;

        HWND hwnd = CreateWindowExW(
            exStyle,
            ClassName,
            title,
            style,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            outerWidth,
            outerHeight,
            nullptr,
            nullptr,
            instance,
            this
        );

        if (hwnd == nullptr)
        {
            Destroy();
            return false;
        }

        m_handle = hwnd;
        m_state.Dpi = GetDpiForWindow(hwnd);
        UpdateClientSize(hwnd);

        return true;
    }

    void NativeWindow::Show(int command) const
    {
        if (m_handle == nullptr)
        {
            return;
        }

        ShowWindow(m_handle, command);
        UpdateWindow(m_handle);
    }

    void NativeWindow::Destroy() noexcept
    {
        if (m_handle != nullptr)
        {
            DestroyWindow(m_handle);
            m_handle = nullptr;
        }

        if (m_classRegistered)
        {
            UnregisterClassW(ClassName, m_instance);
            m_classRegistered = false;
        }

        m_instance = nullptr;
        m_eventHead = 0;
        m_eventCount = 0;
    }

    bool NativeWindow::PollEvent(WindowEvent& event) noexcept
    {
        if (m_eventCount == 0)
        {
            return false;
        }

        event = m_events[m_eventHead];
        m_eventHead =
            (m_eventHead + 1) % EventCapacity;

        --m_eventCount;

        return true;
    }

    void NativeWindow::PushEvent(
        WindowEventType type) noexcept
    {
        if (m_eventCount == EventCapacity)
        {
            m_eventHead =
                (m_eventHead + 1) % EventCapacity;

            --m_eventCount;
        }

        const std::size_t index =
            (m_eventHead + m_eventCount) % EventCapacity;

        m_events[index] = WindowEvent{ type, m_state };
        ++m_eventCount;
    }

    void NativeWindow::UpdateClientSize(HWND hwnd) noexcept
    {
        RECT rect{};

        if (!GetClientRect(hwnd, &rect))
        {
            return;
        }

        m_state.Width = rect.right - rect.left;
        m_state.Height = rect.bottom - rect.top;
    }

    LRESULT CALLBACK NativeWindow::WindowProc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        if (message == WM_NCCREATE)
        {
            const auto* createStruct =
                reinterpret_cast<const CREATESTRUCTW*>(lParam);

            auto* window = static_cast<NativeWindow*>(
                createStruct->lpCreateParams
            );

            if (window == nullptr)
            {
                return FALSE;
            }

            SetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(window)
            );

            window->m_handle = hwnd;

            return DefWindowProcW(
                hwnd,
                message,
                wParam,
                lParam
            );
        }

        auto* window = reinterpret_cast<NativeWindow*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA)
        );

        if (window == nullptr)
        {
            return DefWindowProcW(
                hwnd, message, wParam, lParam);
        }

        return window->HandleMessage(
            hwnd, message, wParam, lParam);
    }

    LRESULT NativeWindow::HandleMessage(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (message)
        {
        case WM_SIZE:
        {
            const bool wasMinimized = m_state.IsMinimized;

            m_state.IsMinimized =
                (wParam == SIZE_MINIMIZED);

            UpdateClientSize(hwnd);

            if (m_state.IsMinimized)
            {
                if (!wasMinimized)
                {
                    PushEvent(WindowEventType::Minimized);
                }
            }
            else
            {
                if (wasMinimized)
                {
                    PushEvent(WindowEventType::Restored);
                }

                PushEvent(WindowEventType::Resized);
            }

            return 0;
        }

        case WM_ENTERSIZEMOVE:
            m_state.IsResizing = true;
            return 0;

        case WM_EXITSIZEMOVE:
            m_state.IsResizing = false;
            UpdateClientSize(hwnd);
            PushEvent(WindowEventType::Resized);
            return 0;

        case WM_DPICHANGED:
        {
            m_state.Dpi = LOWORD(wParam);

            const auto* suggestedRect =
                reinterpret_cast<const RECT*>(lParam);

            if (suggestedRect != nullptr)
            {
                SetWindowPos(
                    hwnd,
                    nullptr,
                    suggestedRect->left,
                    suggestedRect->top,
                    suggestedRect->right -
                        suggestedRect->left,
                    suggestedRect->bottom -
                        suggestedRect->top,
                    SWP_NOZORDER | SWP_NOACTIVATE
                );
            }

            UpdateClientSize(hwnd);
            PushEvent(WindowEventType::DpiChanged);

            return 0;
        }

        case WM_GETMINMAXINFO:
        {
            auto* info =
                reinterpret_cast<MINMAXINFO*>(lParam);

            constexpr int minClientWidth = 640;
            constexpr int minClientHeight = 360;

            RECT rect{
                0, 0,
                minClientWidth,
                minClientHeight
            };

            const UINT dpi = GetDpiForWindow(hwnd);

            if (AdjustWindowRectExForDpi(
                    &rect,
                    static_cast<DWORD>(
                        GetWindowLongPtrW(hwnd, GWL_STYLE)),
                    FALSE,
                    static_cast<DWORD>(
                        GetWindowLongPtrW(hwnd, GWL_EXSTYLE)),
                    dpi))
            {
                info->ptMinTrackSize.x =
                    rect.right - rect.left;

                info->ptMinTrackSize.y =
                    rect.bottom - rect.top;
            }

            return 0;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_NCDESTROY:
        {
            m_handle = nullptr;

            SetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA,
                0
            );

            return DefWindowProcW(
                hwnd, message, wParam, lParam);
        }

        default:
            return DefWindowProcW(
                hwnd, message, wParam, lParam);
        }
    }
}
