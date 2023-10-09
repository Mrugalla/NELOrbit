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

    const auto& user = *audioProcessor.props.getUserSettings();
    const auto width = user.getIntValue("editorWidth", 400);
    const auto height = user.getIntValue("editorHeight", 400);
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

    auto& user = *audioProcessor.props.getUserSettings();
    user.setValue("editorWidth", getWidth());
    user.setValue("editorHeight", getHeight());
}