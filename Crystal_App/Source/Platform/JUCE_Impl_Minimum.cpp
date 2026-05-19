//==============================================================================
#include <iostream>
#include <JuceHeader.h>
#include <streambuf>
#include <string>
#include "Core/Core_Mininum.h"
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

	class JUCE_MainComponent : public juce::Component
	{
	public:
		std::function<void(Core::EventBase&)> send_event;
		void mouseDown(const juce::MouseEvent& e) override
		{
			Core::MouseClickEvent engine_event(e.position.x, e.position.y);
			if (send_event) send_event(engine_event);
		}


	};
	class CRSTWindow : public Core::WindowBase, public juce::DocumentWindow
	{
	public:
		CRSTWindow() : DocumentWindow("JUCE Window", juce::Colours::black, allButtons)
		{
			main_comp = new JUCE_MainComponent();
			setContentOwned(main_comp, true);
			centreWithSize(800, 600);
			setVisible(true);
		}

		void routeEvent(const Core::EventCallbackFn& callback) override 
		{
			main_comp->send_event = callback;
		}

		void closeButtonPressed() override
		{
			Core::AppShouldQuitEvent quit_event;
			if (main_comp->send_event) main_comp->send_event(quit_event);
			if (quit_event.handled)
			{
				std::cout << "[Platform] Engine has quited." << std::endl;
				juce::JUCEApplication::getInstance()->systemRequestedQuit();
			}
		}
	private:
		JUCE_MainComponent* main_comp;
	};
}
namespace Crystal::Core
{
	std::unique_ptr<WindowBase> WindowBase::createWindow()
	{
		return std::make_unique<Platform::CRSTWindow>();
	}
}

class JUCE_Entry : public juce::JUCEApplication
{
public:
	const juce::String getApplicationName() override { return "Crystal"; }
	const juce::String getApplicationVersion() override { return "1.0"; }

	void initialise(const juce::String& commandLine) override
	{
		redirector = std::make_unique<Crystal::Platform::OutputRedirector>();

		main_app.reset(Crystal::Core::createApplication());
		main_app->initialise();
	}
	void shutdown() override
	{
		
	}
private:
	std::unique_ptr<Crystal::Core::ApplicationBase> main_app;
	std::unique_ptr<Crystal::Platform::OutputRedirector> redirector;
};
START_JUCE_APPLICATION(JUCE_Entry)