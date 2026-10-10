#include "Window.h"

#include "Core/Logging/Logger.h"

#include <limits>
#include <utility>

namespace MyGameEngine
{
    Window::~Window()
    {
        Destroy();
    }

    bool Window::Create(
        const WindowProperties& properties)
    {
        if (m_Handle || m_ClassRegistered)
        {
            ENGINE_LOG_ERROR(
                "Window already initialized"
            );
            return false;
        }

        if (properties.Title.empty() ||
            properties.Width == 0 ||
            properties.Height == 0 ||
            properties.Width > 32767 ||
            properties.Height > 32767)
        {
            ENGINE_LOG_ERROR(
                "Invalid window properties"
            );
            return false;
        }

        m_Instance = GetModuleHandleW(nullptr);

        if (!m_Instance)
        {
            ENGINE_LOG_ERROR(
                "GetModuleHandleW failed: {}",
                GetLastError()
            );
            return false;
        }

        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = m_Instance;
        windowClass.hCursor =
            LoadCursorW(nullptr, IDC_ARROW);
        windowClass.hbrBackground =
            static_cast<HBRUSH>(
                GetStockObject(BLACK_BRUSH)
            );
        windowClass.lpszClassName = ClassName;

        if (!RegisterClassExW(&windowClass))
        {
            ENGINE_LOG_ERROR(
                "RegisterClassExW failed: {}",
                GetLastError()
            );

            m_Instance = nullptr;
            return false;
        }

        m_ClassRegistered = true;

        constexpr DWORD style = WS_OVERLAPPEDWINDOW;

        RECT rect
        {
            0,
            0,
            static_cast<LONG>(properties.Width),
            static_cast<LONG>(properties.Height)
        };

        if (!AdjustWindowRectEx(
                &rect,
                style,
                FALSE,
                0))
        {
            ENGINE_LOG_ERROR(
                "AdjustWindowRectEx failed: {}",
                GetLastError()
            );

            Destroy();
            return false;
        }

        m_Handle = CreateWindowExW(
            0,
            ClassName,
            properties.Title.c_str(),
            style,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr,
            nullptr,
            m_Instance,
            this
        );

        if (!m_Handle)
        {
            ENGINE_LOG_ERROR(
                "CreateWindowExW failed: {}",
                GetLastError()
            );

            Destroy();
            return false;
        }

        RECT clientRect{};

        if (!GetClientRect(m_Handle, &clientRect))
        {
            ENGINE_LOG_ERROR(
                "GetClientRect failed: {}",
                GetLastError()
            );

            Destroy();
            return false;
        }

        m_Width = static_cast<std::uint32_t>(
            clientRect.right - clientRect.left
        );

        m_Height = static_cast<std::uint32_t>(
            clientRect.bottom - clientRect.top
        );

        ENGINE_LOG_INFO(
            "Window created: {}x{}",
            m_Width,
            m_Height
        );

        return true;
    }

    void Window::Show() const
    {
        if (!IsValid())
        {
            ENGINE_LOG_WARN(
                "Cannot show invalid window"
            );
            return;
        }

        ShowWindow(m_Handle, SW_SHOW);
        UpdateWindow(m_Handle);

        ENGINE_LOG_INFO("Window shown");
    }

    void Window::Destroy() noexcept
    {
        if (m_Handle)
        {
            HWND handle = m_Handle;

            if (!DestroyWindow(handle))
            {
                ENGINE_LOG_ERROR(
                    "DestroyWindow failed: {}",
                    GetLastError()
                );
            }
        }

        if (m_ClassRegistered)
        {
            if (!UnregisterClassW(
                    ClassName,
                    m_Instance))
            {
                ENGINE_LOG_ERROR(
                    "UnregisterClassW failed: {}",
                    GetLastError()
                );
            }

            m_ClassRegistered = false;
        }

        m_Handle = nullptr;
        m_Instance = nullptr;
        m_Width = 0;
        m_Height = 0;
    }

    bool Window::ProcessMessages()
    {
        MSG message{};

        while (PeekMessageW(
            &message,
            nullptr,
            0,
            0,
            PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                ENGINE_LOG_INFO(
                    "WM_QUIT received"
                );
                return false;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        return true;
    }

    void Window::SetEventCallback(
        EventCallback callback)
    {
        m_EventCallback = std::move(callback);
    }

    HWND Window::GetHandle() const noexcept
    {
        return m_Handle;
    }

    bool Window::IsValid() const noexcept
    {
        return m_Handle &&
            IsWindow(m_Handle) != FALSE;
    }

    std::uint32_t Window::GetWidth() const noexcept
    {
        return m_Width;
    }

    std::uint32_t Window::GetHeight() const noexcept
    {
        return m_Height;
    }

    void Window::DispatchEvent(
        const WindowEvent& event) noexcept
    {
        if (!m_EventCallback)
        {
            return;
        }

        try
        {
            m_EventCallback(event);
        }
        catch (const std::exception& exception)
        {
            ENGINE_LOG_CRITICAL(
                "Window event exception: {}",
                exception.what()
            );

            PostQuitMessage(1);
        }
        catch (...)
        {
            ENGINE_LOG_CRITICAL(
                "Unknown window event exception"
            );

            PostQuitMessage(1);
        }
    }

    LRESULT CALLBACK Window::WindowProc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        if (message == WM_NCCREATE)
        {
            const auto* createStruct =
                reinterpret_cast<CREATESTRUCTW*>(
                    lParam
                );

            auto* window =
                static_cast<Window*>(
                    createStruct->lpCreateParams
                );

            if (!window)
            {
                return FALSE;
            }

            SetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(window)
            );

            window->m_Handle = hwnd;
        }

        auto* window = reinterpret_cast<Window*>(
            GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
            )
        );

        if (window)
        {
            // Do not allow C++ exceptions to escape
            // through the Win32 callback boundary.
            try
            {
                return window->HandleMessage(
                    hwnd,
                    message,
                    wParam,
                    lParam
                );
            }
            catch (...)
            {
                PostQuitMessage(1);
                return 0;
            }
        }

        return DefWindowProcW(
            hwnd,
            message,
            wParam,
            lParam
        );
    }

    LRESULT Window::HandleMessage(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (message)
        {
        case WM_CLOSE:
        {
            ENGINE_LOG_INFO("WM_CLOSE received");

            if (!DestroyWindow(hwnd))
            {
                ENGINE_LOG_ERROR(
                    "DestroyWindow failed: {}",
                    GetLastError()
                );
                PostQuitMessage(1);
            }

            return 0;
        }

        case WM_DESTROY:
        {
            ENGINE_LOG_INFO("WM_DESTROY received");

            WindowEvent event{};
            event.Type = WindowEventType::Closed;

            DispatchEvent(event);

            PostQuitMessage(0);
            return 0;
        }

        case WM_NCDESTROY:
        {
            SetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA,
                0
            );

            m_Handle = nullptr;

            return DefWindowProcW(
                hwnd,
                message,
                wParam,
                lParam
            );
        }

        case WM_SIZE:
        {
            m_Width = static_cast<std::uint32_t>(
                LOWORD(lParam)
            );

            m_Height = static_cast<std::uint32_t>(
                HIWORD(lParam)
            );

            WindowEvent event{};
            event.Type = WindowEventType::Resized;
            event.Width = m_Width;
            event.Height = m_Height;

            DispatchEvent(event);
            return 0;
        }

        case WM_SETFOCUS:
        {
            WindowEvent event{};
            event.Type = WindowEventType::FocusGained;

            DispatchEvent(event);
            return 0;
        }

        case WM_KILLFOCUS:
        {
            WindowEvent event{};
            event.Type = WindowEventType::FocusLost;

            DispatchEvent(event);
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            WindowEvent event{};
            event.Type = WindowEventType::KeyDown;
            event.KeyCode =
                static_cast<std::uint32_t>(wParam);
            event.IsRepeat =
                (lParam & (1LL << 30)) != 0;

            DispatchEvent(event);

            // Preserve default system-key behavior,
            // including Alt+F4.
            if (message == WM_SYSKEYDOWN)
            {
                return DefWindowProcW(
                    hwnd, message, wParam, lParam
                );
            }

            return 0;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            WindowEvent event{};
            event.Type = WindowEventType::KeyUp;
            event.KeyCode =
                static_cast<std::uint32_t>(wParam);

            DispatchEvent(event);

            if (message == WM_SYSKEYUP)
            {
                return DefWindowProcW(
                    hwnd, message, wParam, lParam
                );
            }

            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT paint{};
            BeginPaint(hwnd, &paint);
            EndPaint(hwnd, &paint);
            return 0;
        }

        default:
            return DefWindowProcW(
                hwnd,
                message,
                wParam,
                lParam
            );
        }
    }
}
