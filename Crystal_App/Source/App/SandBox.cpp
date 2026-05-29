//==============================================================================
#include <CRST_Core/CRST_Core.h>
#include <CRST_Framework/CRST_Framework.h>
#include <CRST_Message/CRST_Message.h>
#include <juce_core/juce_core.h>

using namespace Crystal::Framework;
using namespace Crystal::Message;
//==============================================================================
void telemetryInputTest(const Crystal::Input::InputState& current_state) noexcept
{
	static Crystal::Input::InputState last_state{};
	if (current_state.discrete_inputs != last_state.discrete_inputs)
	{
		juce::String log_msg = "[Input Test] Active Keys: ";

		if (current_state.discrete_inputs.test(static_cast<size_t>(Crystal::Input::InputBit::KeyW))) log_msg += "W ";
		if (current_state.discrete_inputs.test(static_cast<size_t>(Crystal::Input::InputBit::KeyA))) log_msg += "A ";
		if (current_state.discrete_inputs.test(static_cast<size_t>(Crystal::Input::InputBit::KeyS))) log_msg += "S ";
		if (current_state.discrete_inputs.test(static_cast<size_t>(Crystal::Input::InputBit::KeyD))) log_msg += "D ";
		if (current_state.discrete_inputs.test(static_cast<size_t>(Crystal::Input::InputBit::KeySpace))) log_msg += "SPACE ";

		const Crystal::CRSTf32 mx = current_state.continuous_inputs[static_cast<size_t>(Crystal::Input::InputChannel::MouseX)];
		const Crystal::CRSTf32 my = current_state.continuous_inputs[static_cast<size_t>(Crystal::Input::InputChannel::MouseY)];
		log_msg += juce::String::formatted("| MousePos: (%.2f, %.2f)", mx, my);
		DBG(log_msg);
	}
	last_state = current_state;
}

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
			DBG("Hello");
		}
		void onDetach() override {}
		void onTimeAdvance(const Time::Duration& duration, const Input::InputState& input) override
		{
			telemetryInputTest(input);

		}
	};
}
namespace Crystal::Client
{
	class Sandbox : public WindowedApplicationBase
	{
	public:
		void userInitialise() override
		{
			layers.push_back(std::make_unique<AppLayer>("Crystal"));
			layers[0]->onAttach();
			std::cout << "[Sandbox] Sandbox initialised" << std::endl;
		}
	protected:
		void userCommandProcess(CommandBase& cmd) override{}
		void userEventProcess(EventBase& e) override {}
		void userTimeAdvance(const Time::Duration duration, const Input::InputState& input) override
		{
			
		}
	};
}
namespace Crystal::Framework
{
    std::unique_ptr<ApplicationBase> createApplication()
    {
        return std::make_unique<Client::Sandbox>();
    }
}