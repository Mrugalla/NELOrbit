#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    props(),
    state("state"),
    params(*this),
    
    dryWet(),
    orbit(11),
    universalBuffer(),
    audioBufs(),
    delays()
#endif
{
    {
        juce::PropertiesFile::Options options;
        options.applicationName = JucePlugin_Name;
        options.filenameSuffix = ".settings";
        options.folderName = "Mrugalla" + juce::File::getSeparatorString() + JucePlugin_Name;
        options.osxLibrarySubFolder = "Application Support";
        options.commonToAllUsers = false;
        options.ignoreCaseOfKeyNames = false;
        options.doNotSave = false;
        options.millisecondsBeforeSaving = 20;
        options.storageFormat = juce::PropertiesFile::storeAsXML;

        props.setStorageParameters(options);
    }

    for (auto p = 0; p < NumPlanetsMacro; ++p)
        orbit.giveBirthWithRandomProperties(p);
}

NELOrbitAudioProcessor::~NELOrbitAudioProcessor()
{
}

const juce::String NELOrbitAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NELOrbitAudioProcessor::acceptsMidi() const
{
    return false;
}

bool NELOrbitAudioProcessor::producesMidi() const
{
    return false;
}

bool NELOrbitAudioProcessor::isMidiEffect() const
{
    return false;
}

double NELOrbitAudioProcessor::getTailLengthSeconds() const
{
    return 0.;
}

int NELOrbitAudioProcessor::getNumPrograms()
{
    return 1;
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

void NELOrbitAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
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
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NELOrbitAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto mainOut = layouts.getMainOutputChannelSet();
    const auto mainIn = layouts.getMainInputChannelSet();

    return (mainOut == mono || mainOut == stereo) && (mainOut == mainIn);
}
#endif

void NELOrbitAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
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

    dryWet.saveDry
    (
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

    dryWet.processWet
    (
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

    orbit.processBlock
    (
        universalBuffer,
        numSamples,
        numPlanets,
        params[param::PID::Gravity].getValDenorm(),
        1.f - params[param::PID::SpaceMud].getValDenorm() * .01f,
        params[param::PID::Attraction].getValDenorm() * .01f
    );

    universalBuffer.makeSmooth
    (
        delays.getRingBufferSizeF(),
        params[param::PID::Depth].getValue(),
        numSamples,
        numPlanets
    );

    delays
    (
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

bool NELOrbitAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NELOrbitAudioProcessor::createEditor()
{
    return new NELOrbitAudioProcessorEditor (*this);
}

void NELOrbitAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    params.savePatch(state);
    orbit.savePatch(state);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NELOrbitAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(state.getType()))
            state = juce::ValueTree::fromXml(*xmlState);
    params.loadPatch(state);
    orbit.loadPatch(state);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NELOrbitAudioProcessor();
}

#undef NumPlanetsMacro