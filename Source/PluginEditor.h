#pragma once

#include "OrbitEditor.h"
#include "UI.h"

#include <JuceHeader.h>
#include "PluginProcessor.h"

class NELOrbitAudioProcessorEditor :
    public juce::AudioProcessorEditor,
    public juce::Timer
{
public:
    using Orbit = NELOrbitAudioProcessor::Orbit;

    NELOrbitAudioProcessorEditor (NELOrbitAudioProcessor&);
    
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    NELOrbitAudioProcessor& audioProcessor;

    orbit::gui::Editor<float, NumPlanetsMacro, 30> orbit;
    
    orbit::gui::Utils utils;
    orbit::gui::UI ui;

    juce::Image bg;
    juce::Random rand;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NELOrbitAudioProcessorEditor)
};

