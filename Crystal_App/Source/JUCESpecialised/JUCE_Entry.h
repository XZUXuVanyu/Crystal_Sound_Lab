//==============================================================================
#pragma once
#include <JuceHeader.h>
#include "JUCE_MainComponent.h"
#include "./Core/Application.h"
//==============================================================================
class JUCE_MainComponent;
class CRSTWindow;
//==============================================================================
class CRSTWindow : public Crystal::Core::WindowBase, public juce::DocumentWindow
{
public:
	void getWidth() const override;
	void getHeight() const override;
	void* getNativePtr() const override;
	void setRefreshRate(const Crystal::CRSTu16& refresh_rate) const override;
	void onUpdate() override;

	//==============================================================================
    CRSTWindow(const Crystal::Core::WindowContext& context)
        : DocumentWindow(context.title, juce::Colours::darkgrey, allButtons),
        WindowBase(context)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new JUCE_MainComponent(), true);

        setResizable(true, true);
        centreWithSize(DocumentWindow::getWidth(), DocumentWindow::getHeight());

        setVisible(true);
    }
    ~CRSTWindow() override;
    //==============================================================================
    void resized() override;
    void closeButtonPressed() override;
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CRSTWindow);
};
class CRSTApp : public Crystal::Core::ApplicationBase<CRSTWindow>
{
public:

    CRSTApp(const Crystal::Core::ApplicationContext& context);
    void initialise(
        const std::optional<Crystal::Core::WindowContext>& window_context) override;
	void onUpdate() override;
    void onRender() override;
   
private:
    Crystal::CRSTbool window_initialised = false;
};
class JUCE_Entry : public juce::JUCEApplication
{
public:
    //==============================================================================
    JUCE_Entry();
    ~JUCE_Entry() override;
    //==============================================================================
    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;
    //==============================================================================
    void initialise(const juce::String& commandLineParameters) override;
    void shutdown() override;
    void systemRequestedQuit() override;
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JUCE_Entry);
    std::unique_ptr<CRSTApp> main_app;

    Crystal::CRSTbool app_initialised = false;
    //==============================================================================
};
