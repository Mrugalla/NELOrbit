#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NELOrbitAudioProcessor::NELOrbitAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    state("state"),
    params(*this),
    
    dryWet(),
    orbit(11),
    universalBuffer(),
    audioBufs(),
    delays()
#endif
{
    //orbit.randomizePlanetPositions();

    using namespace orbit;

    orbit.giveBirthToPlanet(
        { Vec<float>(0.f, 0.f), Vec<float>(0.f, 0.f), 1.7f , .02f },
        0
    );

    orbit.giveBirthToPlanet(
        { Vec<float>(0.f, .5f), Vec<float>(-.001f, 0.f), .4f, .005f },
        1
    );
    orbit.giveBirthToPlanet(
        { Vec<float>(0.f, -.5f), Vec<float>(.001f, 0.f), .4f, .005f },
        2
    );
    orbit.giveBirthToPlanet(
        { Vec<float>(.5f, .5f), Vec<float>(-.001f, -.001f), .4f, .005f },
        3
    );
    orbit.giveBirthToPlanet(
        { Vec<float>(-.5f, -.5f), Vec<float>(-.001f, .001f), .4f, .005f },
        4
    );

    for(auto p = 5; p < NumPlanetsMacro; ++p)
        orbit.giveBirthWithRandomProperties(p);
}

NELOrbitAudioProcessor::~NELOrbitAudioProcessor()
{
}

//==============================================================================
const juce::String NELOrbitAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NELOrbitAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NELOrbitAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NELOrbitAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NELOrbitAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NELOrbitAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NELOrbitAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NELOrbitAudioProcessor::setCurrentProgram (int)
{
}

const juce::String NELOrbitAudioProcessor::getProgramName (int)
{
    return {};
}

void NELOrbitAudioProcessor::changeProgramName (int, const juce::String&)
{
}

//==============================================================================
void NELOrbitAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto sampleRateF = static_cast<float>(sampleRate);
    orbit.prepare(sampleRateF, samplesPerBlock);
    universalBuffer.prepare(sampleRateF, samplesPerBlock);
    delays.prepare(sampleRateF, samplesPerBlock);
    for (auto& b : audioBufs)
        for(auto& ch: b)
            ch.resize(samplesPerBlock, 0);
    dryWet.prepare(sampleRateF, samplesPerBlock);
}

void NELOrbitAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NELOrbitAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NELOrbitAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const auto numSamples = buffer.getNumSamples();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    const auto numChannelsIn = getChannelCountOfBus(true, 0);
    const auto numChannels = buffer.getNumChannels();
    const auto samplesDry = const_cast<float const**>(buffer.getArrayOfReadPointers());
    auto samples = const_cast<float **>(buffer.getArrayOfWritePointers());

    dryWet.saveDry(
        samplesDry,
        params[param::PID::Mix].getValue(),
        numChannelsIn,
        numChannels,
        numSamples
    );

    if (numChannels == 2 && params[param::PID::StereoConfig].getValue() > .5f)
    {
        midSide::encode(samples, numSamples);
        processBlock(samples, numChannels, numSamples);
        midSide::decode(samples, numSamples);
    }
    else
        processBlock(samples, numChannels, numSamples);

    dryWet.processWet(
        samples,
        params[param::PID::Gain].getValDenorm(),
        numChannelsIn,
        numChannels,
        numSamples
    );
    
}
void NELOrbitAudioProcessor::processBlock(float** samples, int numChannels, int numSamples) noexcept
{
    for (auto& audioBuf : audioBufs)
        for (auto ch = 0; ch < numChannels; ++ch)
        {
            auto buf = audioBuf[ch].data();
            for (auto s = 0; s < numSamples; ++s)
                buf[s] = samples[ch][s];
        }

    const auto numPlanets = static_cast<int>(params[param::PID::NumPlanets].getValDenorm() + .5f);

    orbit.processBlock(
        universalBuffer,
        numSamples,
        numPlanets,
        params[param::PID::Gravity].getValDenorm(),
        1.f - params[param::PID::SpaceMud].getValDenorm() * .01f,
        params[param::PID::Attraction].getValDenorm() * .01f
    );

    universalBuffer.makeSmooth(
        delays.getRingBufferSizeF(),
        params[param::PID::Depth].getValue(),
        numSamples,
        numPlanets
    );

    delays(
        audioBufs,
        universalBuffer,
        numPlanets,
        numChannels,
        numSamples
    );

    {
        const auto& audioBuf = audioBufs[0];
        for (auto ch = 0; ch < numChannels; ++ch)
        {
            const auto buf = audioBuf[ch].data();
            for (auto s = 0; s < numSamples; ++s)
                samples[ch][s] = buf[s];
        }
    }
    for (auto i = 1; i < numPlanets; ++i)
    {
        const auto& audioBuf = audioBufs[i];
        for (auto ch = 0; ch < numChannels; ++ch)
        {
            const auto buf = audioBuf[ch].data();
            for (auto s = 0; s < numSamples; ++s)
                samples[ch][s] += buf[s];
        }
    }
    const auto planetsGain = 1.f / std::sqrt(static_cast<float>(numPlanets));
    for (auto ch = 0; ch < numChannels; ++ch)
        juce::FloatVectorOperations::multiply(samples[ch], planetsGain, numSamples);
}

//==============================================================================
bool NELOrbitAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NELOrbitAudioProcessor::createEditor()
{
    return new NELOrbitAudioProcessorEditor (*this);
}

//==============================================================================
void NELOrbitAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    params.savePatch(state);
    orbit.savePatch(state);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NELOrbitAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(state.getType()))
            state = juce::ValueTree::fromXml(*xmlState);
    params.loadPatch(state);
    orbit.loadPatch(state);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NELOrbitAudioProcessor();
}

#undef NumPlanetsMacro