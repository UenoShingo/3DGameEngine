#include "Window.h"

#include "Core/Logging/Logger.h"

#include <limits>

namespace MyGameEngine
{
    Window::~Window()
    {
        Destroy();
    }

    bool Window::Create(
        const WindowProperties& properties)
    {
        if (m_Handle != nullptr || m_ClassRegistered)
        {
            ENGINE_LOG_ERROR(
                "Window is already initialized"
            );
            return false;
        }

        if (properties.Title.empty() ||
            properties.Width == 0 ||
            properties.Height == 0 ||
            properties.Width >
                static_cast<std::uint32_t>(
                    (std::numeric_limits<int>::max)()) ||
            properties.Height >
                static_cast<std::uint32_t>(
                    (std::numeric_limits<int>::max)()))
        {
            ENGINE_LOG_ERROR(
                "Invalid window properties"
            );
            return false;
        }

        m_Instance = GetModuleHandleW(nullptr);

        if (m_Instance == nullptr)
        {
            ENGINE_LOG_ERROR(
                "GetModuleHandleW failed: {}",
                GetLastError()
            );
            return false;
        }

        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(WNDCLASSEXW);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = m_Instance;
        windowClass.hCursor = LoadCursorW(
            nullptr,
            IDC_ARROW
        );
        windowClass.hbrBackground =
            static_cast<HBRUSH>(
                GetStockObject(BLACK_BRUSH)
            );
        windowClass.lpszClassName = ClassName;

        if (RegisterClassExW(&windowClass) == 0)
        {
            ENGINE_LOG_ERROR(
                "RegisterClassExW failed: {}",
                GetLastError()
            );

            m_Instance = nullptr;
            return false;
        }

        m_ClassRegistered = true;

        constexpr DWORD windowStyle =
            WS_OVERLAPPEDWINDOW;

        RECT windowRect{
            0,
            0,
            static_cast<LONG>(properties.Width),
            static_cast<LONG>(properties.Height)
        };

        if (!AdjustWindowRectEx(
                &windowRect,
                windowStyle,
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

        const int windowWidth =
            windowRect.right - windowRect.left;

        const int windowHeight =
            windowRect.bottom - windowRect.top;

        m_Handle = CreateWindowExW(
            0,
            ClassName,
            properties.Title.c_str(),
            windowStyle,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowWidth,
            windowHeight,
            nullptr,
            nullptr,
            m_Instance,
            nullptr
        );

        if (m_Handle == nullptr)
        {
            ENGINE_LOG_ERROR(
                "CreateWindowExW failed: {}",
                GetLastError()
            );

            Destroy();
            return false;
        }

        m_Width = properties.Width;
        m_Height = properties.Height;

        ENGINE_LOG_INFO(
            "Window created: {}x{}",
            m_Width,
            m_Height
        );

        return true;
    }

    void Window::Show() const
    {
        if (m_Handle == nullptr)
        {
            ENGINE_LOG_WARN(
                "Cannot show an invalid window"
            );
            return;
        }

        ShowWindow(m_Handle, SW_SHOW);
        UpdateWindow(m_Handle);

        ENGINE_LOG_INFO("Window shown");
    }

    void Window::Destroy() noexcept
    {
        if (m_Handle != nullptr)
        {
            if (!DestroyWindow(m_Handle))
            {
                ENGINE_LOG_ERROR(
                    "DestroyWindow failed: {}",
                    GetLastError()
                );
            }
            else
            {
                ENGINE_LOG_INFO("Window destroyed");
            }

            m_Handle = nullptr;
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

        m_Instance = nullptr;
        m_Width = 0;
        m_Height = 0;
    }

    HWND Window::GetHandle() const noexcept
    {
        return m_Handle;
    }

    bool Window::IsValid() const noexcept
    {
        return m_Handle != nullptr &&
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

    LRESULT CALLBACK Window::WindowProc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        return DefWindowProcW(
            hwnd,
            message,
            wParam,
            lParam
        );
    }
}
