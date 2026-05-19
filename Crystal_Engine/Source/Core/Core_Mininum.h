//==============================================================================
// 文件名: EngineCore.h (归属于 CrystalEngine 项目)
//==============================================================================
#pragma once
#include <functional>
#include <memory>
#include <iostream>
#include "Event.h"
#include "Layer.h"
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
        void quit();
        //==============================================================================
        virtual void onEvent(EventBase& e)
        {
            EventDispatcher dispatcher{ e };
            dispatcher.dispatch<WindowClose>(CRST_BIND_EVENT_CALLBACK(onWindowClose));
            if (e.handled) return;
            for (auto layer_iter = layers.rbegin(); layer_iter != layers.rend(); ++layer_iter)
            {
                if (e.handled) break;
                (*layer_iter)->onEvent(e);
            }
        }
        template<isCRSTLayer Layer_T>
        void pushLayer(std::unique_ptr<Layer_T> layer)
        {
            layers.emplace_back(std::move(layer));
        }
        //==============================================================================
        virtual CRSTbool onWindowClose(WindowClose& e) = 0;
        //==============================================================================
    protected:
        std::unique_ptr<WindowBase> window;
        std::vector<std::unique_ptr<LayerBase>> layers;
    };
    std::unique_ptr<ApplicationBase> createApplication();
}