#include "PluginProcessor.h"
#include "PluginEditor.h"

NELOrbitAudioProcessorEditor::NELOrbitAudioProcessorEditor(NELOrbitAudioProcessor& p) :
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

    const auto& state = audioProcessor.state;
    const auto width = state.getProperty("editorWidth", 400);
    const auto height = state.getProperty("editorHeight", 400);
    setSize(width, height);
}

void NELOrbitAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void NELOrbitAudioProcessorEditor::resized()
{
    orbit.setBounds(getLocalBounds());
    ui.setBounds(getLocalBounds().reduced(2));

    audioProcessor.state.setProperty("editorWidth", getWidth(), nullptr);
    audioProcessor.state.setProperty("editorHeight", getHeight(), nullptr);
}