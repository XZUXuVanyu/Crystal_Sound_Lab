//==============================================================================
// 文件名: EngineCore.h (归属于 CrystalEngine 项目)
//==============================================================================
#pragma once
#include <functional>
#include <memory>
#include <iostream>
namespace Crystal::Core
{
    class EventBase
	{
    public:
        bool handled = false;
        virtual ~EventBase() = default;
    };
    struct MouseClickEvent : public EventBase
	{
        float x, y;
        MouseClickEvent(float x, float y) : x(x), y(y) {}
    };
    struct AppShouldQuitEvent : EventBase
    {
        AppShouldQuitEvent() {}
    };

    using EventCallbackFn = std::function<void(EventBase&)>;
    class WindowBase
	{
    public:
        virtual ~WindowBase() = default;
        virtual void routeEvent(const EventCallbackFn& callback) = 0;

        static std::unique_ptr<WindowBase> createWindow();
    };
    class ApplicationBase
	{
    public:
        virtual ~ApplicationBase() = default;
        virtual void initialise()
    	{
            window = WindowBase::createWindow();
            window->routeEvent([this](EventBase& e) { this->onEvent(e); });

            std::cout << "[Engine] Engine initialised" << std::endl;
        }
        virtual void onEvent(EventBase& e) 
    	{
            if (auto* quit_event = dynamic_cast<AppShouldQuitEvent*>(&e))
            {
                std::cout << "[Core] Engine requested quit..." << std::endl ;
                quit_event->handled = true;
            }
            if (auto* mouse_event = dynamic_cast<MouseClickEvent*>(&e)) 
            {
                std::cout << "[Core] Mouse Click: X="
                    << mouse_event->x << " Y=" << mouse_event->y << std::endl;
            }
        }
        void quit();
    protected:
        std::unique_ptr<WindowBase> window;
    };
    ApplicationBase* createApplication();
}