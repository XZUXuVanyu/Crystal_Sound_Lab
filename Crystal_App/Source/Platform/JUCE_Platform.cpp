//==============================================================================
#include <chrono>
#include <iostream>
#include <streambuf>
#include <string>
#include <JuceHeader.h>
#include "Core/Application.h"
#include "Core/Event.h"
#include "Core/Timer.h"
#include "Core/Window.h"
//==============================================================================
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
			original_buf = std::cout.rdbuf();
			std::cout.rdbuf(&juce_buf);
		}
		~OutputRedirector()
		{
			std::cout.rdbuf(original_buf);
		}
	private:
		OutputStreamBuf juce_buf;
		std::streambuf* original_buf = nullptr;
	};
}
namespace Crystal::Platform
{
	class JUCE_Functionality : public juce::Component
	{
	public:
		std::function<void(Core::EventBase&)> comp_dispatch_event_callback = nullptr;
		void mouseDown(const juce::MouseEvent& event) override
		{
			Core::MouseButtonCode button = Core::MouseButtonCode::None;
			if (event.mods.isLeftButtonDown()) button = Core::MouseButtonCode::Left;
			if (event.mods.isRightButtonDown()) button = Core::MouseButtonCode::Right;
			if (event.mods.isMiddleButtonDown()) button = Core::MouseButtonCode::Middle;

			Core::MouseButtonPressed engine_event(button);
			if (comp_dispatch_event_callback) comp_dispatch_event_callback(engine_event);
		}
		void mouseMove(const juce::MouseEvent& event) override
		{
			CRSTf32 x, y;
			x = static_cast<CRSTf32>(event.x) / static_cast<CRSTf32>(getWidth());
			y = static_cast<CRSTf32>(event.y) / static_cast<CRSTf32>(getHeight());

			Core::MouseMoved engine_event(x, y);
			if (comp_dispatch_event_callback) comp_dispatch_event_callback(engine_event);
		}
	};
}
namespace Crystal::Platform
{
	class CRST_Timer : public juce::Timer, public Core::FixedTimeStepBase
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
	class CRST_Window : public Core::ApplicationWindowBase, public juce::DocumentWindow
	{
	public:
		CRST_Window() : DocumentWindow("JUCE Window", juce::Colours::black, allButtons)
		{
			component = new JUCE_Functionality();
			setContentOwned(component, true);
			centreWithSize(800, 600);
			setVisible(true);
		}
		void routeEvent(const std::function<void(Core::EventBase&)>& callback) override
		{
			// ApplicationBase::onEvent -> dispatch_event_callback
			dispatch_event_callback = callback;
			component->comp_dispatch_event_callback = dispatch_event_callback;
		}
		void closeButtonPressed() override
		{
			std::cout << "[Platform] Requesting quit..." << std::endl;
			Core::WindowClose quit_event;
			if (dispatch_event_callback) dispatch_event_callback(quit_event);
			if (quit_event.handled)
			{
				std::cout << "[Platform] Engine has quited" << std::endl;
				juce::JUCEApplication::getInstance()->systemRequestedQuit();
			}
		}
	private:
		JUCE_Functionality* component;
	};
}
namespace Crystal::Core
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

		main_app = Crystal::Core::createApplication();
		main_app->initialise(0.01666667);
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
	std::unique_ptr<Crystal::Core::MinimumApplicationBase> main_app;
	std::unique_ptr<Crystal::Platform::OutputRedirector> redirector;
	//==============================================================================
};
START_JUCE_APPLICATION(JUCE_Entry)
//==============================================================================