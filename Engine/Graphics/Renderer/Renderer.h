#pragma once

#include <array>

namespace MyGameEngine
{
    class GraphicsContext;
    class SwapChain;

    class Renderer final
    {
    public:
        Renderer() = default;

        [[nodiscard]]
        bool RenderFrame(
            GraphicsContext& context,
            SwapChain& swapChain
        );

        void SetClearColor(
            float red,
            float green,
            float blue,
            float alpha = 1.0f
        ) noexcept;

        void SetVSync(bool enabled) noexcept;

        [[nodiscard]]
        bool IsVSyncEnabled() const noexcept;

    private:
        std::array<float, 4> m_ClearColor{
            0.09f, 0.14f, 0.22f, 1.0f
        };

        bool m_VSyncEnabled = true;
    };
}
