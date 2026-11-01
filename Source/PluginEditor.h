#pragma once
#include "PluginProcessor.h"

class S950Look : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
};

class S950FilterEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit S950FilterEditor (S950FilterProcessor&);
    ~S950FilterEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { repaint(); }
    S950FilterProcessor& proc;
    S950Look look;
    juce::Slider knob;
    juce::AudioProcessorValueTreeState::SliderAttachment att;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (S950FilterEditor)
};
