/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NELOrbitAudioProcessorEditor::NELOrbitAudioProcessorEditor (NELOrbitAudioProcessor& p) :
    AudioProcessorEditor(&p),
    audioProcessor(p),
    orbit(p.orbit)
{
    addAndMakeVisible(orbit);
    setOpaque(true);
    setResizable(true, true);
    setSize(900, 900);
}

NELOrbitAudioProcessorEditor::~NELOrbitAudioProcessorEditor()
{
}

//==============================================================================
void NELOrbitAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void NELOrbitAudioProcessorEditor::resized()
{
    orbit.setBounds(gui::maxQuadIn(getLocalBounds().toFloat()).toNearestInt());
}
