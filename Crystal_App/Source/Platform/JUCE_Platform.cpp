//==============================================================================
#include <chrono>
#include <iostream>
#include <JuceHeader.h>
#include <streambuf>
#include <string>

#include <CRST_Core/CRST_Core.h>
#include <CRST_Framework/CRST_Framework.h>
#include <CRST_Message/CRST_Message.h>
#include <CRST_Time/CRST_Time.h>
//==============================================================================
using namespace Crystal;
using namespace Crystal::Framework;
using namespace Crystal::Time;
namespace Crystal::Platform
{
	class OutputStreamBuf : public std::streambuf
	{
	protected:
		std::streamsize xsputn(const char_type* s, std::streamsize count) override
		{
			for (std::streamsize i = 0; i < count; ++i)
			{
				buffer += s[i];
				if (s[i] == '\n')
				{
					flushToJuce();
				}
			}
			return count;
		}
		int_type overflow(int_type c = traits_type::eof()) override
		{
			if (c != traits_type::eof())
			{
				buffer += traits_type::to_char_type(c);
				if (c == '\n')
				{
					flushToJuce();
				}
			}
			return c;
		}
	private:
		void flushToJuce()
		{
			if (!buffer.empty() && buffer.back() == '\n')
			{
				buffer.pop_back();
			}
			juce::Logger::writeToLog(juce::String(buffer));
			buffer.clear();
		}
		std::string buffer;
	};
	class OutputRedirector
	{
	public:
		OutputRedirector()
		{
			original_cout_buf = std::cout.rdbuf();
			original_cerr_buf = std::cerr.rdbuf();
			std::cout.rdbuf(&juce_buf);
			std::cerr.rdbuf(&juce_buf);
		}
		~OutputRedirector()
		{
			std::cout.rdbuf(original_cout_buf);
			std::cerr.rdbuf(original_cerr_buf);
		}
	private:
		OutputStreamBuf juce_buf;
		std::streambuf* original_cout_buf = nullptr;
		std::streambuf* original_cerr_buf = nullptr;
	};
}
namespace Crystal::Platform
{
	class JUCE_Functionality : public juce::Component
	{
	public:
		std::function<void(Message::EventBase&)> comp_dispatch_event_callback = nullptr;
	};
}
namespace Crystal::Platform
{
	class CRST_InputAdapter : public Input::InputBase,
		public juce::KeyListener, public juce::MouseListener
	{
	public:
		//==============================================================================
		explicit CRST_InputAdapter(CRSTu64 capacity = 256)
			: InputBase(capacity){}
		~CRST_InputAdapter() override = default;
		CRST_NON_COPYABLE(CRST_InputAdapter)
		//==============================================================================
		bool keyPressed(const juce::KeyPress&, juce::Component*) override
		{
			return false;
		}
		bool keyStateChanged(bool isKeyDown, juce::Component*) override
		{
			this->recordDiscreteInput(Input::DiscreteInputEvent{.absolute_time_nano = 0, .bit = Input::InputBit::KeyW,
				.is_down = juce::KeyPress::isKeyCurrentlyDown('w') || juce::KeyPress::isKeyCurrentlyDown('W') });
			this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::KeyA,
				.is_down = juce::KeyPress::isKeyCurrentlyDown('a') || juce::KeyPress::isKeyCurrentlyDown('A') });
			this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::KeyS,
				.is_down = juce::KeyPress::isKeyCurrentlyDown('s') || juce::KeyPress::isKeyCurrentlyDown('S') });
			this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::KeyD,
				.is_down = juce::KeyPress::isKeyCurrentlyDown('d') || juce::KeyPress::isKeyCurrentlyDown('D') });

			this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::KeySpace,
				.is_down = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::spaceKey) });
			this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::KeyEscape,
				.is_down = juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::escapeKey) });

			const auto modifiers = juce::ModifierKeys::getCurrentModifiers();

			this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::KeyShift,
				.is_down = modifiers.isShiftDown() });
			this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::KeyCtrl,
				.is_down = modifiers.isCtrlDown() });
			this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::KeyAlt,
				.is_down = modifiers.isAltDown() });

			return true;
		}
		void mouseMove(const juce::MouseEvent& event) override
		{
			this->recordContinuousInput(
				Input::ContinuousInputEvent{ .absolute_time_nano = 0 ,
					.channel = Input::InputChannel::MouseX, .value = static_cast<CRSTf32>(event.x) });
			this->recordContinuousInput(
				Input::ContinuousInputEvent{ .absolute_time_nano = 0 ,
					.channel = Input::InputChannel::MouseY, .value = static_cast<CRSTf32>(event.y) });
		}
		void mouseDrag(const juce::MouseEvent& event) override
		{
			mouseMove(event);
		}
		void mouseDown(const juce::MouseEvent& event) override
		{
			if (event.mods.isLeftButtonDown())   
				this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::MouseLeft,
				.is_down = true });
			if (event.mods.isRightButtonDown())
				this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::MouseRight,
				.is_down = true });
			if (event.mods.isMiddleButtonDown())
				this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::MouseMiddle,
				.is_down = true });
		}
		void mouseUp(const juce::MouseEvent& event) override
		{
			if (event.mods.isLeftButtonDown())
				this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::MouseLeft,
				.is_down = false });
			if (event.mods.isRightButtonDown())
				this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::MouseRight,
				.is_down = false });
			if (event.mods.isMiddleButtonDown())
				this->recordDiscreteInput(Input::DiscreteInputEvent{ .absolute_time_nano = 0, .bit = Input::InputBit::MouseMiddle,
				.is_down = false });
		}
	};
	class CRST_Timer : public TimerBase
	{
	public:
		CRSTu64 getAbsoluteTimeNano() noexcept override
		{
			return static_cast<CRSTu64>(juce::Time::getHighResolutionTicks()) * 100ULL;
		}
	};
	class CRST_Window : public ApplicationWindowBase, public juce::DocumentWindow
	{
	public:
		CRST_Window() : DocumentWindow("JUCE Window", juce::Colours::black, allButtons)
		{
			component = new JUCE_Functionality();
			setContentOwned(component, true);
			centreWithSize(800, 600);
			setVisible(true);
		}
		void routeEvent(const std::function<void(Message::EventBase&)>& callback) override
		{
			// ApplicationBase::onEvent -> dispatch_event_callback
			dispatch_event_callback = callback;
			component->comp_dispatch_event_callback = dispatch_event_callback;
		}
		void linkInputAdapter(Input::InputBase* input_adapter) noexcept override
		{
			CRST_ASSERT(input_adapter != nullptr, "Initialise input_adapter first");

			auto* juce_adapter = static_cast<CRST_InputAdapter*>(input_adapter);

			component->addMouseListener(juce_adapter, true);
			component->addKeyListener(juce_adapter);
			component->setWantsKeyboardFocus(true);
			component->grabKeyboardFocus();
		}
		void closeButtonPressed() override
		{
			std::cout << "[Platform] Requesting quit..." << std::endl;
			Message::submitCommand<Message::ApplicationShutDown>();
			if (!ApplicationBase::isRunning())
			{
				juce::JUCEApplication::getInstance()->systemRequestedQuit();
			}
		}	  

	private:
		JUCE_Functionality* component;
	};
}
namespace Crystal::Framework
{
	std::unique_ptr<WindowBase> createWindow()
	{
		return std::make_unique<Platform::CRST_Window>();
	}
}
namespace Crystal::Time
{
	std::unique_ptr<TimerBase> createTimer()
	{
		return std::make_unique<Platform::CRST_Timer>();
	}
	std::unique_ptr<ClockBase> createClock(CRSTf64 time_step)
	{
		return std::make_unique<ClockBase>(time_step, createTimer());
	}
}
namespace Crystal::Input
{
	std::unique_ptr<InputBase> createInputAdapter(CRSTu64 capacity)
	{
		return std::make_unique<Platform::CRST_InputAdapter>(capacity);
	}
}

class JUCE_Entry : public juce::JUCEApplication
{
public:
	//==============================================================================
	const juce::String getApplicationName() override { return "Crystal"; }
	const juce::String getApplicationVersion() override { return "1.0"; }
	//==============================================================================
	void initialise(const juce::String& commandLine) override
	{
		redirector = std::make_unique<Crystal::Platform::OutputRedirector>();

		main_app = createApplication();
		main_app->initialise(ApplicationContext{
		.name = "Sandbox" , .version = "0.0.1", .type = Framework::ApplicationType::Windowed,
			.time_step = 0.01, .input_buffer_capacity = 256 });
	}
	void shutdown() override
	{
		std::cout << "[Platform] Cleaning up..." << std::endl;
		main_app.reset();
		std::cout << "[Platform] Done" << std::endl;
		redirector.reset();
	}
	//==============================================================================
private:
	//==============================================================================
	std::unique_ptr<Framework::ApplicationBase> main_app;
	std::unique_ptr<Platform::OutputRedirector> redirector;
	//==============================================================================
};
START_JUCE_APPLICATION(JUCE_Entry)
//==============================================================================