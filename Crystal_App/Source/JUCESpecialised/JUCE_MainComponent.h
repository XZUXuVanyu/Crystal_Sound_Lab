//==============================================================================
#pragma once
#include <JuceHeader.h>
//==============================================================================
class JUCE_MainComponent : public juce::Component
{
public:
    //==============================================================================
    JUCE_MainComponent() = default;
    ~JUCE_MainComponent() override = default;
    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::white);
        g.drawText("Hello", getLocalBounds(), true);
    }
    void resized() override
    {
        setBoundsRelative(0.25, 0.25, 
            0.5, 0.5);
    }
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JUCE_MainComponent);
};