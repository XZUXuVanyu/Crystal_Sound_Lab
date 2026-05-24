//==============================================================================
#include <CRST_Core/CRST_Core.h>
#include <CRST_Framework/CRST_Framework.h>
#include <CRST_Message/CRST_Message.h>
using namespace Crystal::Framework;
using namespace Crystal::Message;
//==============================================================================
namespace Crystal::Client
{
	class AppLayer : public LayerBase
	{
	public:
		AppLayer(std::string app_name) : LayerBase(std::move(app_name)) {}
		void onEvent(EventBase& e) override
		{
			EventDispatcher dispatcher{e};
			auto checked =
			dispatcher.dispatch<MouseButtonPressed>([this](MouseButtonPressed& event)
			{
				std::cout << "MouseButtonPressed: button = " <<
					static_cast<int>(event.button) << std::endl;
				return true;
			});
		}

		void onAttach() override
		{
			std::cout << "[Layer: App] onAttach()" << std::endl;
		}
		void onDetach() override {}
		void onTimeAdvance(CRSTf64 dt) override {}
	};
}
namespace Crystal::Client
{
	class Sandbox : public WindowedApplicationBase
	{
	public:
		void userInitialise() override
		{
			submitCommand<LayerPush>(std::make_unique<AppLayer>("Crystal"));
			std::cout << "[Sandbox] Sandbox initialised" << std::endl;
		}
	protected:
		void userTimeAdvance(CRSTf64 dt) override{}
		void userCommandProcess(CommandBase& cmd) override{}
		void userEventProcess(EventBase& e) override {}
	};
}
namespace Crystal::Framework
{
    std::unique_ptr<MinimumApplicationBase> createApplication()
    {
        return std::make_unique<Client::Sandbox>();
    }
}