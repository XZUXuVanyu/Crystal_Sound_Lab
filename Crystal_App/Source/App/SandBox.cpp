#include "Core/Application.h"
using namespace Crystal::Core;
namespace Crystal::Client
{
	class AppLayer : public LayerBase
	{
	public:
		AppLayer() : LayerBase("App") {}
		void onEvent(EventBase& e) override
		{
			EventDispatcher dispatcher{e};
			dispatcher.dispatch<MouseButtonPressed>([this](MouseButtonPressed& event)
			{
				std::cout << "[Layer: App] MouseButtonPressed: button = " <<
					static_cast<int>(event.button) << std::endl;
				return true;
			});
		}
	};
	class UIOverlayLayer : public Crystal::Core::LayerBase
	{
	public:
		UIOverlayLayer() : LayerBase("UI_Overlay") {}
		void onEvent(EventBase& e) override
		{
			EventDispatcher dispatcher{e};
			dispatcher.dispatch<MouseButtonPressed>([this](MouseButtonPressed& event)
			{
				std::cout << "[Layer: UI] MouseButtonPressed" << std::endl;
				return false;
			});
		}
	};
}
namespace Crystal::Client
{
	class Sandbox : public WindowedApplicationBase
	{
	public:
		void initialise(CRSTf64 time_step) override
		{
			WindowedApplicationBase::initialise(time_step);
			pushLayer(std::make_unique<AppLayer>());
			pushLayer(std::make_unique<UIOverlayLayer>());

			std::cout << "[Sandbox] Sandbox initialised" << std::endl;
		}
		void onEvent(EventBase& e) override
		{
			WindowedApplicationBase::onEvent(e);
		}
	};
}
namespace Crystal::Core
{
    std::unique_ptr<MinimumApplicationBase> createApplication()
    {
        return std::make_unique<Client::Sandbox>();
    }
}