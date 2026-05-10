//==============================================================================
#include <JuceHeader.h>
#include "JUCE_Entry.h"
#include "./Core/Utilities.h"
//==============================================================================
using namespace std;

void CRSTWindow::getWidth() const
{
}

void CRSTWindow::getHeight() const
{
}

void* CRSTWindow::getNativePtr() const
{
    return nullptr;
}

void CRSTWindow::setRefreshRate(const Crystal::CRSTu16& refresh_rate) const
{
}

void CRSTWindow::onUpdate()
{
}

CRSTWindow::~CRSTWindow()
{
}

void CRSTWindow::resized()
{
    setSize(DocumentWindow::getWidth(), DocumentWindow::getHeight());
}

void CRSTWindow::closeButtonPressed()
{
    JUCE_Entry::getInstance()->systemRequestedQuit();
}

CRSTApp::CRSTApp(const Crystal::Core::ApplicationContext& context)
	:ApplicationBase(context)
{

}

void CRSTApp::initialise(const std::optional<Crystal::Core::WindowContext>& window_context)
{
	if (window_context.has_value() && !window_initialised)
	{
        this->host_window = make_unique<CRSTWindow>(*window_context);
	}
    window_initialised = true;
}

void CRSTApp::onUpdate()
{
}

void CRSTApp::onRender()
{
}

JUCE_Entry::JUCE_Entry()
{
}

JUCE_Entry::~JUCE_Entry()
{
}

const juce::String JUCE_Entry::getApplicationName()
{
    CRST_RUNTIME_CHECK(app_initialised && main_app, "Initialise JUCE_Entry first", "default");
    return main_app->getContext().name;
}
const juce::String JUCE_Entry::getApplicationVersion()
{
    CRST_RUNTIME_CHECK(app_initialised && main_app, "Initialise JUCE_Entry first", "default");
    return main_app->getContext().version;
}
void JUCE_Entry::initialise(const juce::String& commandLineParameters)
{
    CRST_RUNTIME_CHECK(!app_initialised && !main_app, "main_app already initialised");
    main_app = make_unique<CRSTApp>(Crystal::Core::ApplicationContext("Crystal","0.0.1"));
    main_app->initialise(Crystal::Core::WindowContext("Hello",1080,760,0));
}

void JUCE_Entry::shutdown()
{
    main_app = nullptr;
    quit();
}

void JUCE_Entry::systemRequestedQuit()
{
    main_app->requestQuit();
    shutdown();
}
