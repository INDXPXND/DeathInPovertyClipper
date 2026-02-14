#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ClipperAudioProcessorEditor::ClipperAudioProcessorEditor(ClipperAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Загрузка фонового изображения
    loadBackgroundImage();
    
    // Настройка стандартного LookAndFeel для mix слайдера
    customLookAndFeel.setColour(juce::Slider::thumbColourId, juce::Colours::red);
    customLookAndFeel.setColour(juce::Slider::trackColourId, juce::Colours::whitesmoke);
    customLookAndFeel.setColour(juce::Slider::backgroundColourId, juce::Colours::whitesmoke.withAlpha(0.3f));

    // ========== Threshold Slider (КРАСНЫЙ) ==========
    thresholdSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    thresholdSlider.setLookAndFeel(&thresholdLookAndFeel);  // Красный стиль
    addAndMakeVisible(thresholdSlider);
    
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centred);
    thresholdLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    thresholdLabel.attachToComponent(&thresholdSlider, false);
    addAndMakeVisible(thresholdLabel);
    
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, ClipperAudioProcessor::PARAM_THRESHOLD, thresholdSlider);

    // ========== Clip Type ComboBox ==========
    clipTypeCombo.addItem("Hard Clip", 1);
    clipTypeCombo.addItem("Soft Clip", 2);
    addAndMakeVisible(clipTypeCombo);
    
    clipTypeLabel.setText("Clip Type", juce::dontSendNotification);
    clipTypeLabel.setJustificationType(juce::Justification::centred);
    clipTypeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    clipTypeLabel.attachToComponent(&clipTypeCombo, false);
    addAndMakeVisible(clipTypeLabel);
    
    clipTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.parameters, ClipperAudioProcessor::PARAM_CLIP_TYPE, clipTypeCombo);

    // ========== Input Gain Slider (ЗЕЛЕНЫЙ) ==========
    inputGainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    inputGainSlider.setLookAndFeel(&inputGainLookAndFeel);  // Зеленый стиль
    addAndMakeVisible(inputGainSlider);
    
    inputGainLabel.setText("Input Gain", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    inputGainLabel.attachToComponent(&inputGainSlider, false);
    addAndMakeVisible(inputGainLabel);
    
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, ClipperAudioProcessor::PARAM_INPUT_GAIN, inputGainSlider);

    // ========== Output Gain Slider (ЗЕЛЕНЫЙ) ==========
    outputGainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    outputGainSlider.setLookAndFeel(&outputGainLookAndFeel);  // Зеленый стиль
    addAndMakeVisible(outputGainSlider);
    
    outputGainLabel.setText("Output Gain", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    outputGainLabel.attachToComponent(&outputGainSlider, false);
    addAndMakeVisible(outputGainLabel);
    
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, ClipperAudioProcessor::PARAM_OUTPUT_GAIN, outputGainSlider);

    // ========== Mix Slider ==========
    mixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    mixSlider.setLookAndFeel(&customLookAndFeel);  // Стандартный стиль
    addAndMakeVisible(mixSlider);
    
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centredLeft);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    mixLabel.attachToComponent(&mixSlider, true);
    addAndMakeVisible(mixLabel);
    
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, ClipperAudioProcessor::PARAM_MIX, mixSlider);

    // Размер окна
    setSize(370, 570);
    
    // Запуск таймера для обновления UI
    startTimerHz(30);
}

ClipperAudioProcessorEditor::~ClipperAudioProcessorEditor()
{
    // Очищаем LookAndFeel перед удалением
    thresholdSlider.setLookAndFeel(nullptr);
    inputGainSlider.setLookAndFeel(nullptr);
    outputGainSlider.setLookAndFeel(nullptr);
    mixSlider.setLookAndFeel(nullptr);
}

//==============================================================================
void ClipperAudioProcessorEditor::loadBackgroundImage()
{
    juce::File imageFile("C:/Users/pronk/JUCE_projects/DeathInPovertyClipper/Source/02-death-in-poverty.png");
    
    if (imageFile.existsAsFile())
    {
        backgroundImage = juce::ImageFileFormat::loadFrom(imageFile);
        DBG("Background image loaded successfully");
    }
    else
    {
        DBG("Background image file not found: " + imageFile.getFullPathName());
    }
}

//==============================================================================
void ClipperAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Отрисовка фонового изображения
    if (backgroundImage.isValid())
    {
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    }
    else
    {
        // Запасной вариант: градиентный фон
        juce::ColourGradient gradient(
            juce::Colour(0xff1a1a2e), 0, 0,
            juce::Colour(0xff16213e), 0, static_cast<float>(getHeight()),
            false
        );
        g.setGradientFill(gradient);
        g.fillAll();
    }
}

void ClipperAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Верхняя секция с контролами клиппинга
    auto topSection = bounds.removeFromTop(220);
    
    thresholdSlider.setBounds(62, 18, 248, 248);
    inputGainSlider.setBounds(17, 390, 180, 180);
    outputGainSlider.setBounds(175, 390, 180, 180);
    clipTypeCombo.setBounds(260, 245, 100, 20);
    
    // Нижняя секция с mix slider
    bounds.removeFromTop(20);
    auto mixSection = bounds.removeFromTop(130);
    mixSection.removeFromTop(15);
    mixSection.removeFromLeft(50);
    mixSlider.setBounds(mixSection.removeFromLeft(mixSection.getWidth() - 10));
}

void ClipperAudioProcessorEditor::timerCallback()
{
    repaint();
}
