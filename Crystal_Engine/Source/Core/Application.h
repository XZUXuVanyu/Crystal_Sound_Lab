//==============================================================================
#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include "Event.h"
#include "Layer.h"
#include "Timer.h"
#include "Window.h"
//==============================================================================
namespace Crystal::Core
{
    class MinimumApplicationBase
    {
    public:
        using is_application = void;
        //------------------------------------------------------------------------------
        virtual ~MinimumApplicationBase() = default;
        MinimumApplicationBase(const MinimumApplicationBase&) = delete;
        MinimumApplicationBase& operator=(const MinimumApplicationBase&) = delete;
        //------------------------------------------------------------------------------
        virtual void initialise(CRSTf64 time_step) = 0;
        virtual void onTimeAdvance(CRSTf64 dt) = 0;
        virtual void onEvent(EventBase& e) = 0;
        //------------------------------------------------------------------------------
        template<isCRSTLayer Layer_T>
        void pushLayer(std::unique_ptr<Layer_T> layer)
        {
            layers.emplace_back(std::move(layer));
        }
    protected:
        MinimumApplicationBase() = default;
    protected:
        std::vector<std::unique_ptr<LayerBase>> layers;
        CRSTbool is_running = true;
    };
}
namespace Crystal::Core
{
    class WindowedApplicationBase : public MinimumApplicationBase
    {
    public:
        //==============================================================================
        /// <summary>
        /// Initialize the application
        /// </summary>
        /// <remarks>
        /// This method creates the main application window and routes native system events 
        /// to the underlying event system. It must be invoked manually within the 
        /// platform-specific application.
        /// </remarks>
        void        initialise(CRSTf64 time_step) override
        {
            timer = createTimer(time_step);
            timer->start(CRST_BIND_EVENT_CALLBACK(onTimeAdvance));

            window = createWindow();
            window->routeEvent(CRST_BIND_EVENT_CALLBACK(onEvent));

            std::cout << "[Engine] Engine initialised" << std::endl;
        }
        //==============================================================================
        void        onTimeAdvance(CRSTf64 dt) noexcept override
        {
            for (auto layer_it = layers.begin(); layer_it != layers.end(); ++layer_it)
            {
                (*layer_it)->onTimeAdvance(dt);
            }
        }
        void        onEvent(EventBase& e) override
        {
            EventDispatcher dispatcher{ e };
            dispatcher.dispatch<WindowClose>(CRST_BIND_EVENT_CALLBACK(onWindowClose));

            for (auto layer_it = layers.rbegin(); layer_it != layers.rend(); ++layer_it)
            {
                if (e.handled) break;
                (*layer_it)->onEvent(e);
            }
        }
        virtual CRSTbool    onWindowClose(WindowClose& e)
        {
            std::cout << "[Engine] WindowClose event handled" << std::endl;
            e.handled = true;
            return true;
        }
    protected:
        WindowedApplicationBase() = default;
    protected:
        std::unique_ptr<MinimumWindowBase> window;
        std::unique_ptr<MinimumTimerBase> timer;
    };

    /// TODO: class TaskApplicationBase : public MinimumApplicationBase

}
namespace Crystal::Core
{
    std::unique_ptr<MinimumApplicationBase> createApplication();
}
//==============================================================================