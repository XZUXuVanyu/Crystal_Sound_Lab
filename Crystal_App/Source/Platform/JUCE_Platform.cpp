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
		void mouseDown(const juce::MouseEvent& event) override
		{
			Message::MouseButtonCode button = Message::MouseButtonCode::None;
			if (event.mods.isLeftButtonDown()) button = Message::MouseButtonCode::Left;
			if (event.mods.isRightButtonDown()) button = Message::MouseButtonCode::Right;
			if (event.mods.isMiddleButtonDown()) button = Message::MouseButtonCode::Middle;

			Message::raiseEvent<Message::MouseButtonPressed>(button);
		}
		void mouseMove(const juce::MouseEvent& event) override
		{
			CRSTf32 x, y;
			x = static_cast<CRSTf32>(event.x) / static_cast<CRSTf32>(getWidth());
			y = static_cast<CRSTf32>(event.y) / static_cast<CRSTf32>(getHeight());

			Message::raiseEvent<Message::MouseMoved>(x, y);
		}
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
			this->recordDiscreteInput(Input::InputBit::KeyW, juce::KeyPress::isKeyCurrentlyDown('w') || juce::KeyPress::isKeyCurrentlyDown('W'));
			this->recordDiscreteInput(Input::InputBit::KeyA, juce::KeyPress::isKeyCurrentlyDown('a') || juce::KeyPress::isKeyCurrentlyDown('A'));
			this->recordDiscreteInput(Input::InputBit::KeyS, juce::KeyPress::isKeyCurrentlyDown('s') || juce::KeyPress::isKeyCurrentlyDown('S'));
			this->recordDiscreteInput(Input::InputBit::KeyD, juce::KeyPress::isKeyCurrentlyDown('d') || juce::KeyPress::isKeyCurrentlyDown('D'));

			this->recordDiscreteInput(Input::InputBit::KeySpace, juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::spaceKey));
			this->recordDiscreteInput(Input::InputBit::KeyEscape, juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::escapeKey));
			this->recordDiscreteInput(Input::InputBit::KeyEnter, juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::returnKey));

			const auto modifiers = juce::ModifierKeys::getCurrentModifiers();
			this->recordDiscreteInput(Input::InputBit::KeyShift, modifiers.isShiftDown());
			this->recordDiscreteInput(Input::InputBit::KeyCtrl, modifiers.isCtrlDown());
			this->recordDiscreteInput(Input::InputBit::KeyAlt, modifiers.isAltDown());

			return true;
		}
		void mouseMove(const juce::MouseEvent& event) override
		{
			this->recordContinuousInput(Input::InputChannel::MouseX, static_cast<CRSTf32>(event.x));
			this->recordContinuousInput(Input::InputChannel::MouseY, static_cast<CRSTf32>(event.y));
		}
		void mouseDrag(const juce::MouseEvent& event) override
		{
			mouseMove(event);
		}
		void mouseDown(const juce::MouseEvent& event) override
		{
			if (event.mods.isLeftButtonDown())   this->recordDiscreteInput(Input::InputBit::MouseLeft, true);
			if (event.mods.isRightButtonDown())  this->recordDiscreteInput(Input::InputBit::MouseRight, true);
			if (event.mods.isMiddleButtonDown()) this->recordDiscreteInput(Input::InputBit::MouseMiddle, true);
		}
		void mouseUp(const juce::MouseEvent& event) override
		{
			if (!event.mods.isLeftButtonDown())   
				this->recordDiscreteInput(Input::InputBit::MouseLeft, false);
			if (!event.mods.isRightButtonDown())  
				this->recordDiscreteInput(Input::InputBit::MouseRight, false);
			if (!event.mods.isMiddleButtonDown()) 
				this->recordDiscreteInput(Input::InputBit::MouseMiddle, false);
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
		void closeButtonPressed() override
		{
			std::cout << "[Platform] Requesting quit..." << std::endl;
			Message::submitCommand<Message::ApplicationShutDown>();
			if (!Framework::ApplicationBase::isRunning())
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