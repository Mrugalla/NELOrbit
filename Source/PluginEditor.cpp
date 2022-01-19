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
    juce::Timer(),
    audioProcessor(p),
    orbit(p.orbit),
    utils(*this, p.params),
    ui(utils),
    bg(juce::Image::ARGB, 1, 1, false)
{
    bg.setPixelAt(0, 0, juce::Colours::transparentBlack);

    addAndMakeVisible(orbit);
    addAndMakeVisible(ui);

    setOpaque(true);
    setResizable(true, true);
    startTimerHz(12);
    setSize(400, 400);
}

void NELOrbitAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.drawImageAt(bg, orbit.getX(), orbit.getY(), false);
}

void NELOrbitAudioProcessorEditor::resized()
{
    orbit.setBounds(getLocalBounds());
    ui.setBounds(getLocalBounds().reduced(2));
}

void NELOrbitAudioProcessorEditor::timerCallback()
{
    if (orbit.getWidth() == orbit.getHeight())
        bg = bg.rescaled(orbit.getWidth(), orbit.getHeight(), juce::Graphics::ResamplingQuality::highResamplingQuality);

    const auto width = static_cast<float>(bg.getWidth() * 2);
    
    juce::Graphics g{ bg };
    {
        const auto sat = .12f;
        const auto lum = .12f;
        const auto alpha = .12f;
        const auto w = rand.nextFloat() * width;
        const juce::Rectangle<float> cloudBounds(
            rand.nextFloat() * width - width,
            rand.nextFloat() * width - width,
            w, w
        );
        const auto rad = w * .5f;
        const juce::ColourGradient gradient(
            juce::Colour::fromHSL(
                rand.nextFloat(),
                sat,
                lum,
                alpha
            ),
            {
                cloudBounds.getX() + rad,
                cloudBounds.getY() + rad
            },
            juce::Colours::transparentBlack,
            cloudBounds.getTopLeft(),
            true
        );
        g.setGradientFill(gradient);
        g.fillRect(cloudBounds);
    }
    
    repaint();
}