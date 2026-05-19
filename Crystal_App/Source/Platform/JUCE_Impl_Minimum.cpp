//==============================================================================
#include <iostream>
#include <JuceHeader.h>
#include <streambuf>
#include <string>
#include "Core/Core_Mininum.h"
#include "Core/Event.h"
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
		void mouseDown(const juce::MouseEvent& event) override
		{
			Core::MouseButtonCode button = Core::MouseButtonCode::None;
			if (event.mods.isLeftButtonDown()) button = Core::MouseButtonCode::Left;
			if (event.mods.isRightButtonDown()) button = Core::MouseButtonCode::Right;
			if (event.mods.isMiddleButtonDown()) button = Core::MouseButtonCode::Middle;

			Core::MouseButtonPressed engine_event(button);
			if (send_event) send_event(engine_event);
		}
		void mouseMove(const juce::MouseEvent& event) override
		{
			CRSTf32 x, y;
			x = static_cast<CRSTf32>(event.x) / static_cast<CRSTf32>(getWidth());
			y = static_cast<CRSTf32>(event.y) / static_cast<CRSTf32>(getHeight());

			Core::MouseMoved engine_event(x, y);
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
		void routeEvent(const std::function<void(Core::EventBase&)>& callback) override
		{
			main_comp->send_event = callback;
		}
		void closeButtonPressed() override
		{
			std::cout << "[Platform] Requesting quit..." << std::endl;
			Core::WindowClose quit_event;
			if (main_comp->send_event) main_comp->send_event(quit_event);
			if (quit_event.handled)
			{
				std::cout << "[Platform] Engine has quited" << std::endl;
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
	//==============================================================================
	const juce::String getApplicationName() override { return "Crystal"; }
	const juce::String getApplicationVersion() override { return "1.0"; }
	//==============================================================================
	void initialise(const juce::String& commandLine) override
	{
		redirector = std::make_unique<Crystal::Platform::OutputRedirector>();

		main_app = Crystal::Core::createApplication();
		main_app->initialise();
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
	std::unique_ptr<Crystal::Core::ApplicationBase> main_app;
	std::unique_ptr<Crystal::Platform::OutputRedirector> redirector;
	//==============================================================================
};
START_JUCE_APPLICATION(JUCE_Entry)