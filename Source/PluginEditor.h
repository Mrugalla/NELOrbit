#pragma once

#include "OrbitEditor.h"
#include "UI.h"

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NELOrbitAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    using Orbit = NELOrbitAudioProcessor::Orbit;

    NELOrbitAudioProcessorEditor (NELOrbitAudioProcessor&);
    ~NELOrbitAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    NELOrbitAudioProcessor& audioProcessor;

    orbit::gui::Editor<float, NumPlanetsMacro, 30> orbit;
    
    orbit::gui::Utils utils;
    orbit::gui::UI ui;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NELOrbitAudioProcessorEditor)
};

