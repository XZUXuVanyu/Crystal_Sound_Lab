//==============================================================================
// 文件名: EngineCore.h (归属于 CrystalEngine 项目)
//==============================================================================
#pragma once
#include <functional>
#include <memory>
#include <iostream>
#include "Event.h"
//==============================================================================
namespace Crystal::Core
{
    class WindowBase
	{
    public:
        //==============================================================================
        virtual ~WindowBase() = default;
        virtual void routeEvent(const std::function<void(EventBase&)>& callback) = 0;
        //==============================================================================
        static std::unique_ptr<WindowBase> createWindow();
    };
    class ApplicationBase
	{
    public:
        //==============================================================================
        virtual ~ApplicationBase() = default;
        //==============================================================================
        virtual void initialise()
    	{
            window = WindowBase::createWindow();
            window->routeEvent([this](EventBase& e) { this->onEvent(e); });

            std::cout << "[Engine] Engine initialised" << std::endl;
        }
        virtual void onEvent(EventBase& e) 
    	{
            EventDispatcher dispatcher{ e };
            dispatcher.dispatch<WindowClose>(CRST_BIND_EVENT_CALLBACK(onWindowClose));
            dispatcher.dispatch<MouseMoved>(CRST_BIND_EVENT_CALLBACK(onMouseMoved));
            dispatcher.dispatch<MouseButtonPressed>(CRST_BIND_EVENT_CALLBACK(onMouseButtonPressed));
        }
        void quit();
        //==============================================================================
        virtual CRSTbool onWindowClose(WindowClose& e) = 0;
        virtual CRSTbool onMouseMoved(MouseMoved& e) = 0;
        virtual CRSTbool onMouseButtonPressed(MouseButtonPressed& e) = 0;
    protected:
        std::unique_ptr<WindowBase> window;
    };
    ApplicationBase* createApplication();
}