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
	class ApplicationBase
	{
    public:
        //==============================================================================
        ApplicationBase(const ApplicationBase&) = delete;
        ApplicationBase& operator=(const ApplicationBase&) = delete;
		virtual ~ApplicationBase() = default;
        //==============================================================================
        /// <summary>
        /// Initialize the application
        /// </summary>
        /// <remarks>
        /// This method creates the main application window and routes native system events 
		/// to the underlying event system. It must be invoked manually within the 
		/// platform-specific application.
        /// </remarks>
        virtual void        initialise(CRSTf64 time_step)
    	{
            timer = createTimer(time_step);
            timer->start(CRST_BIND_EVENT_CALLBACK(onTimeAdvance));

            window = createWindow();
            window->routeEvent(CRST_BIND_EVENT_CALLBACK(onEvent));

            std::cout << "[Engine] Engine initialised" << std::endl;
        }
        //==============================================================================
        virtual void        onTimeAdvance(CRSTf64 dt) noexcept
        {
	        for (auto layer_it = layers.begin(); layer_it != layers.end(); ++layer_it)
	        {
                (*layer_it)->onTimeAdvance(dt);
	        }
        }
        virtual void        onEvent(EventBase& e)
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
        //==============================================================================
        /// <summary>
        /// Push a layer backward into <see cref="layers"/>
        /// </summary>
        /// <param name="layer"> <see cref="std::unique_ptr"/> of the Layer to be pushed </param>
        template<isCRSTLayer Layer_T>
        void                pushLayer(std::unique_ptr<Layer_T> layer)
        {
            layers.emplace_back(std::move(layer));
        }
    protected:
        //==============================================================================
        ApplicationBase() = default;
    protected:
        //==============================================================================
        std::unique_ptr<MinimumWindowBase> window;
        std::unique_ptr<MinimumTimerBase> timer;
        std::vector<std::unique_ptr<LayerBase>> layers;
    };
}
namespace Crystal::Core
{
    std::unique_ptr<ApplicationBase> createApplication();
}
//==============================================================================