#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "LabelColour1LookAndFeel.h"
class BeatSettingsComponent
    : public juce::Component
{

public:

    BeatSettingsComponent();
    ~BeatSettingsComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setBpm(double bpm);
private:


    juce::Label bpmLabel;
    juce::Label currentBpmValueLabel;

    LabelColour1LookAndFeel labelColour1LookAndFeel;

};


