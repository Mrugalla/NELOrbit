#pragma once
#include "Param.h"
#include "Orbit.h"
#include "MidSideEncoder.h"
#include "DryWetProcessor.h"
#include <JuceHeader.h>

struct NELOrbitAudioProcessor :
    public juce::AudioProcessor
{
    using AppProps = juce::ApplicationProperties;

    //==============================================================================
    NELOrbitAudioProcessor();
    ~NELOrbitAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (float** samples, int numChannels, int numSamples) noexcept;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    using DryBuf = std::array<std::vector<float>, 2>;
    using ParamBuf = std::vector<float>;
    using ParamSmooth = orbit::Smooth<float>;

    using Orbit = orbit::Processor<float, NumPlanetsMacro>;
    using UniversalBuffer = orbit::UniversalBuffer<float, NumPlanetsMacro>;
    using AudioBufs = std::array<std::array<std::vector<float>, 2>, NumPlanetsMacro>;
    using Delays = orbit::Delays<float, NumPlanetsMacro>;

    AppProps props;
    juce::ValueTree state;
    param::Params params;
    drywet::Processor dryWet;
    Orbit orbit;
    UniversalBuffer universalBuffer;
    AudioBufs audioBufs;
    Delays delays;
};
