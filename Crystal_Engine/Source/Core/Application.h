//==============================================================================
#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "Layer.h"
#include "Window.h"

//==============================================================================
namespace Crystal::Core
{
	struct ApplicationContext
	{
		std::string name = "Crystal App", version = "0.0.1";
		WindowContext window_context;
	};

	template<CRSTisDerivedFrom<WindowBase> WindowType>
	class ApplicationBase
	{
	public:
		//==============================================================================
		ApplicationBase() = delete;
		virtual ~ApplicationBase() {}
		ApplicationBase(const ApplicationBase& other) = delete;
		ApplicationBase& operator=(const ApplicationBase& other) = delete;
		ApplicationBase(ApplicationBase&& other) = delete;
		ApplicationBase& operator=(ApplicationBase&& other) = delete;
		//==============================================================================
		virtual void initialise(const std::optional<WindowContext>& window_context) {}
		/* TODO: Add logic here */
		void run() {}
		void stop() {}
		virtual void requestQuit() {}
		void shutDown() {}
		//==============================================================================
		virtual void onUpdate() = 0;
		virtual void onRender() = 0;
		//==============================================================================
		ApplicationContext getContext() const
		{
			return context;
		}
		//==============================================================================
		/* TODO: make this a queued operation after event system */
		template<CRSTisDerivedFrom<LayerBase> LayerType>
		void addLayer()
		{
			layer_stack.push_back(std::make_unique<LayerType>());
		}
		/* TODO: add this after event system */
		void removeLayer() {};
		//==============================================================================
	protected:
		//==============================================================================
		ApplicationBase(const ApplicationContext& application_context)
			:context(application_context)
		{}
		//==============================================================================
		std::vector<std::unique_ptr<LayerBase>>& getLayerStack()
		{
			return layer_stack;
		}
	protected:
		//==============================================================================
		std::unique_ptr<WindowType> host_window;
	private:
		//==============================================================================
		ApplicationContext context;
		std::vector<std::unique_ptr<LayerBase>> layer_stack;
	};
}
//==============================================================================