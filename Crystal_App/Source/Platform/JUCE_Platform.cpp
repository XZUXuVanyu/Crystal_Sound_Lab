//==============================================================================
#include <chrono>
#include <iostream>
#include <JuceHeader.h>
#include <streambuf>
#include <string>
#include "Framework/Application.h"
#include "Framework/Timer.h"
#include "Framework/Window.h"
#include "Message/MessageBus.h"
//==============================================================================
using namespace Crystal;
Framework::MinimumApplicationBase* Framework::MinimumApplicationBase::instance = nullptr;

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
	class CRST_Timer : public juce::Timer, public Framework::FixedTimeStepBase
	{
	public:
		CRST_Timer(CRSTf64 time_step) : FixedTimeStepBase(time_step)
		{

		}
		void timerCallback() override
		{
			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<CRSTf64> elapsed = now - last_hardware_time;
			last_hardware_time = now;
			FixedTimeStepBase::onTimeAdvance(elapsed.count());
			// notify_time_advance_callback(time_step) -> ApplicationBase::onTimeAdvance(time_step)
		}
		void start(const std::function<void(CRSTf64)>& callback) override
		{
			// ApplicationBase::onTimeAdvance -> notify_time_advance_callback
			notify_time_advance_callback = callback;
			last_hardware_time = std::chrono::high_resolution_clock::now();

			int interval_ms = static_cast<int>(time_step * 1000.0);
			if (interval_ms < 1) interval_ms = 1;

			startTimer(interval_ms);
		}
		void pause() override
		{
			stopTimer();
		}
		void stop() override
		{
			stopTimer();

			accumulator = 0.0;
			delta_time = 0.0;
			total_time = 0.0;
		}
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> last_hardware_time;
	};
	class CRST_Window : public Framework::ApplicationWindowBase, public juce::DocumentWindow
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
			if (!Framework::MinimumApplicationBase::isRunning())
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
	std::unique_ptr<MinimumWindowBase> createWindow()
	{
		return std::make_unique<Platform::CRST_Window>();
	}
	std::unique_ptr<MinimumTimerBase> createTimer(CRSTf64 time_step)
	{
		return std::make_unique<Platform::CRST_Timer>(time_step);
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

		main_app = Framework::createApplication();
		main_app->initialise(Framework::ApplicationContext{
		.name = "Sandbox" , .version = "0.0.1", .type = Framework::ApplicationType::Windowed,
			.time_step = 0.01 });
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
	std::unique_ptr<Framework::MinimumApplicationBase> main_app;
	std::unique_ptr<Platform::OutputRedirector> redirector;
	//==============================================================================
};
START_JUCE_APPLICATION(JUCE_Entry)
//==============================================================================