#pragma once

namespace MyGameEngine
{
    class Application
    {
    public:
        Application() = default;
        virtual ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

        int Run();
        void RequestExit();

        [[nodiscard]] bool IsRunning() const noexcept;

    protected:
        virtual bool OnInitialize() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnShutdown() = 0;

    private:
        bool m_IsRunning = false;
    };
}
