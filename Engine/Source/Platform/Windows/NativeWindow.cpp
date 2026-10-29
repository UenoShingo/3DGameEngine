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

        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(WNDCLASSEXW);
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

        RECT clientRect{};
        clientRect.right = width;
        clientRect.bottom = height;

        if (!AdjustWindowRectEx(
                &clientRect,
                WS_OVERLAPPEDWINDOW,
                FALSE,
                0))
        {
            Destroy();
            return false;
        }

        const int windowWidth =
            clientRect.right - clientRect.left;

        const int windowHeight =
            clientRect.bottom - clientRect.top;

        m_handle = CreateWindowExW(
            0,
            ClassName,
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowWidth,
            windowHeight,
            nullptr,
            nullptr,
            instance,
            this
        );

        if (m_handle == nullptr)
        {
            Destroy();
            return false;
        }

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
                reinterpret_cast<CREATESTRUCTW*>(lParam);

            auto* window = static_cast<NativeWindow*>(
                createStruct->lpCreateParams
            );

            SetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(window)
            );

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

        switch (message)
        {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_NCDESTROY:
        {
            if (window != nullptr)
            {
                window->m_handle = nullptr;
            }

            SetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA,
                0
            );

            break;
        }

        default:
            break;
        }

        return DefWindowProcW(
            hwnd,
            message,
            wParam,
            lParam
        );
    }
}
