#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * Clipper VST Plugin Processor
 * Обеспечивает hard/soft clipping аудиосигнала с регулируемым порогом
 */
class ClipperAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    ClipperAudioProcessor();
    ~ClipperAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Параметры плагина
    juce::AudioProcessorValueTreeState parameters;
    
    // Идентификаторы параметров
    static constexpr const char* PARAM_THRESHOLD = "threshold";
    static constexpr const char* PARAM_CLIP_TYPE = "clipType";
    static constexpr const char* PARAM_INPUT_GAIN = "inputGain";
    static constexpr const char* PARAM_OUTPUT_GAIN = "outputGain";
    static constexpr const char* PARAM_MIX = "mix";

private:
    //==============================================================================
    // Функции клиппинга
    float hardClip(float sample, float threshold);
    float softClip(float sample, float threshold);
    
    // Буферы для dry/wet микширования
    juce::AudioBuffer<float> dryBuffer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipperAudioProcessor)
};
