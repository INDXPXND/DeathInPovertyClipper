#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ClipperAudioProcessor::ClipperAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#else
    :
#endif
    parameters(*this, nullptr, juce::Identifier("ClipperParams"),
    {
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_THRESHOLD,
            "Threshold",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            1.0f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 2); }
        ),
        
        std::make_unique<juce::AudioParameterChoice>(
            PARAM_CLIP_TYPE,
            "Clip Type",
            juce::StringArray{"Hard", "Soft"},
            0
        ),
        
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_INPUT_GAIN,
            "Input Gain",
            juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
            0.0f,
            "dB",
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " dB"; }
        ),
        
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_OUTPUT_GAIN,
            "Output Gain",
            juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
            0.0f,
            "dB",
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " dB"; }
        ),
        
        std::make_unique<juce::AudioParameterFloat>(
            PARAM_MIX,
            "Mix",
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
            100.0f,
            "%",
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 0) + " %"; }
        )
    })
{
}

ClipperAudioProcessor::~ClipperAudioProcessor()
{
}

//==============================================================================
const juce::String ClipperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ClipperAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ClipperAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double ClipperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ClipperAudioProcessor::getNumPrograms()
{
    return 1;
}

int ClipperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ClipperAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String ClipperAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void ClipperAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void ClipperAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate);
    dryBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
}

void ClipperAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ClipperAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void ClipperAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Очистка лишних каналов
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Получение значений параметров
    const float threshold = parameters.getRawParameterValue(PARAM_THRESHOLD)->load();
    const int clipType = static_cast<int>(parameters.getRawParameterValue(PARAM_CLIP_TYPE)->load());
    const float inputGainDb = parameters.getRawParameterValue(PARAM_INPUT_GAIN)->load();
    const float outputGainDb = parameters.getRawParameterValue(PARAM_OUTPUT_GAIN)->load();
    const float mix = parameters.getRawParameterValue(PARAM_MIX)->load() / 100.0f;

    const float inputGain = juce::Decibels::decibelsToGain(inputGainDb);
    const float outputGain = juce::Decibels::decibelsToGain(outputGainDb);

    // Сохранение сухого сигнала для mix
    dryBuffer.makeCopyOf(buffer, true);

    // Обработка каждого канала
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Применение входного усиления
            float processedSample = channelData[sample] * inputGain;

            // Применение клиппинга
            if (clipType == 0) // Hard clip
            {
                processedSample = hardClip(processedSample, threshold);
            }
            else // Soft clip
            {
                processedSample = softClip(processedSample, threshold);
            }

            // Применение выходного усиления
            processedSample *= outputGain;

            // Применение mix (dry/wet)
            channelData[sample] = processedSample * mix + 
                                  dryBuffer.getSample(channel, sample) * (1.0f - mix);
        }
    }
}

//==============================================================================
float ClipperAudioProcessor::hardClip(float sample, float threshold)
{
    if (sample > threshold)
        return threshold;
    else if (sample < -threshold)
        return -threshold;
    else
        return sample;
}

float ClipperAudioProcessor::softClip(float sample, float threshold)
{
    if (threshold < 0.01f)
        threshold = 0.01f;

    // Tanh-based soft clipping
    float normalizedSample = sample / threshold;
    float clipped = std::tanh(normalizedSample);
    return clipped * threshold;
}

//==============================================================================
bool ClipperAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ClipperAudioProcessor::createEditor()
{
    return new ClipperAudioProcessorEditor(*this);
}

//==============================================================================
void ClipperAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ClipperAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClipperAudioProcessor();
}
