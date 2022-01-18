#pragma once

#include "OrbitEditor.h"

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUIBasics.h"

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

    orbit::gui::Editor<double, NumPlanetsMacro, 30> orbit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NELOrbitAudioProcessorEditor)
};

#undef NumPlanetsMacro