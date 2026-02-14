#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Кастомный LookAndFeel для круглых слайдеров
 */
class CustomRotarySliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomRotarySliderLookAndFeel(juce::Colour activeColour)
        : activeLineColour(activeColour)
    {
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = juce::jmin(8.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;

        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();

        // Фоновая дуга (неактивная часть)
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(centreX, centreY,
                                   arcRadius, arcRadius,
                                   0.0f,
                                   rotaryStartAngle, rotaryEndAngle,
                                   true);

        // Используем цвет активной линии для фона, но с прозрачностью
        g.setColour(activeLineColour.withAlpha(0.3f));
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Активная дуга (заполненная часть)
        if (slider.isEnabled())
        {
            juce::Path valueArc;
            valueArc.addCentredArc(centreX, centreY,
                                  arcRadius, arcRadius,
                                  0.0f,
                                  rotaryStartAngle, toAngle,
                                  true);

            // Активная линия
            g.setColour(activeLineColour);
            g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Указатель (точка на конце)
        juce::Point<float> thumbPoint(
            centreX + arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi),
            centreY + arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi)
        );

        g.setColour(activeLineColour);
        g.fillEllipse(juce::Rectangle<float>(lineW * 1.5f, lineW * 1.5f).withCentre(thumbPoint));
    }

private:
    juce::Colour activeLineColour;
};

//==============================================================================
/**
 * GUI редактор для Clipper плагина
 */
class ClipperAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    ClipperAudioProcessorEditor(ClipperAudioProcessor&);
    ~ClipperAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    
private:
    void timerCallback() override;
    void loadBackgroundImage();
    
    ClipperAudioProcessor& audioProcessor;

    // UI компоненты
    juce::Slider thresholdSlider;
    juce::Label thresholdLabel;
    
    juce::ComboBox clipTypeCombo;
    juce::Label clipTypeLabel;
    
    juce::Slider inputGainSlider;
    juce::Label inputGainLabel;
    
    juce::Slider outputGainSlider;
    juce::Label outputGainLabel;
    
    juce::Slider mixSlider;
    juce::Label mixLabel;
    
    // Attachments для связи с параметрами
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> clipTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    // Look and Feel - отдельный для каждого слайдера
    CustomRotarySliderLookAndFeel thresholdLookAndFeel{juce::Colours::red};      // Красный
    CustomRotarySliderLookAndFeel inputGainLookAndFeel{juce::Colours::lime};     // Зеленый
    CustomRotarySliderLookAndFeel outputGainLookAndFeel{juce::Colours::lime};    // Зеленый
    juce::LookAndFeel_V4 customLookAndFeel;  // Для mix slider
    
    // Фоновое изображение
    juce::Image backgroundImage;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipperAudioProcessorEditor)
};
