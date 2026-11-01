#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// Modèle du filtre de l'Akai S950 : National MF6CN-50, passe-bas Butterworth
// 6e ordre à capacités commutées (36 dB/oct), sans résonance, fc = fclk / 50.
class S950FilterProcessor : public juce::AudioProcessor
{
public:
    S950FilterProcessor();
    ~S950FilterProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "S950 Filter"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    static float knobToCutoffHz (float knob);   // knob 0..99 -> Hz

    juce::AudioProcessorValueTreeState apvts;

private:
    struct Biquad { double b0=1,b1=0,b2=0,a1=0,a2=0; double z1[2]{},z2[2]{}; };
    Biquad sec[3];
    double fs = 44100.0, clockPhase = 0.0;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> cutoff;
    void setCoeffs (double fc);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (S950FilterProcessor)
};
