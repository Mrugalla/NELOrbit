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
    orbit(p.orbit),
    utils(*this, p.params),
    ui(utils)
{
    addAndMakeVisible(orbit);
    addAndMakeVisible(ui);

    setOpaque(true);
    setResizable(true, true);
    setSize(400, 400);
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
    orbit.setBounds(getLocalBounds());
    ui.setBounds(getLocalBounds().reduced(2));
}
