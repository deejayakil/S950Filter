#include "PluginProcessor.h"
#include "PluginEditor.h"

static constexpr float kMinHz = 300.0f;    // plancher réel du S950 (~300 Hz, anti clock-bleed)
static constexpr float kMaxHz = 20000.0f;
static constexpr double kClockRatio = 50.0; // MF6CN-50 : fclk = 50 x fc
static constexpr float kBleedGain = 0.0005f; // ~ -66 dB, estimation

S950FilterProcessor::S950FilterProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE",
             { std::make_unique<juce::AudioParameterFloat> (
                   juce::ParameterID { "filter", 1 }, "Filter",
                   juce::NormalisableRange<float> (0.0f, 99.0f, 1.0f), 99.0f) })
{}

float S950FilterProcessor::knobToCutoffHz (float k)
{
    return kMinHz * std::pow (kMaxHz / kMinHz, juce::jlimit (0.0f, 99.0f, k) / 99.0f);
}

bool S950FilterProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == l.getMainInputChannelSet()
        && (l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
         || l.getMainOutputChannelSet() == juce::AudioChannelSet::mono());
}

void S950FilterProcessor::prepareToPlay (double sr, int)
{
    fs = sr;
    for (auto& s : sec) { std::fill (std::begin (s.z1), std::end (s.z1), 0.0);
                          std::fill (std::begin (s.z2), std::end (s.z2), 0.0); }
    auto hz = knobToCutoffHz (apvts.getRawParameterValue ("filter")->load());
    cutoff.reset (sr, 0.02);
    cutoff.setCurrentAndTargetValue (hz);
    clockPhase = 0.0;
    setCoeffs (hz);
}

// Butterworth 6e ordre = 3 sections, Q = 1/(2 sin((2k-1)pi/12))
void S950FilterProcessor::setCoeffs (double fc)
{
    fc = juce::jmin (fc, fs * 0.45);
    static const double Q[3] = { 1.9318516525781366, 0.7071067811865476, 0.5176380902050415 };
    const double w0 = juce::MathConstants<double>::twoPi * fc / fs;
    const double c = std::cos (w0), s = std::sin (w0);
    for (int i = 0; i < 3; ++i)
    {
        const double alpha = s / (2.0 * Q[i]);
        const double a0 = 1.0 + alpha;
        sec[i].b0 = ((1.0 - c) * 0.5) / a0;
        sec[i].b1 = (1.0 - c) / a0;
        sec[i].b2 = sec[i].b0;
        sec[i].a1 = (-2.0 * c) / a0;
        sec[i].a2 = (1.0 - alpha) / a0;
    }
}

void S950FilterProcessor::processBlock (juce::AudioBuffer<float>& buf, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    const int nCh = juce::jmin (buf.getNumChannels(), 2), n = buf.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c) buf.clear (c, 0, n);

    cutoff.setTargetValue (knobToCutoffHz (apvts.getRawParameterValue ("filter")->load()));

    for (int i = 0; i < n; ++i)
    {
        if ((i & 15) == 0 || cutoff.isSmoothing()) {}
        const float fc = cutoff.getNextValue();
        if ((i & 7) == 0) setCoeffs (fc);

        // fuite d'horloge : carré à fclk = 50 x fc (repliement naturel = sifflement qui suit la coupure)
        clockPhase += kClockRatio * fc / fs;
        clockPhase -= std::floor (clockPhase);
        const float bleed = (clockPhase < 0.5 ? 1.0f : -1.0f) * kBleedGain;

        for (int ch = 0; ch < nCh; ++ch)
        {
            double x = buf.getSample (ch, i);
            for (auto& s : sec)
            {
                const double y = s.b0 * x + s.z1[ch];
                s.z1[ch] = s.b1 * x - s.a1 * y + s.z2[ch];
                s.z2[ch] = s.b2 * x - s.a2 * y;
                x = y;
            }
            buf.setSample (ch, i, (float) x + bleed);
        }
    }
}

juce::AudioProcessorEditor* S950FilterProcessor::createEditor() { return new S950FilterEditor (*this); }

void S950FilterProcessor::getStateInformation (juce::MemoryBlock& d)
{ if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, d); }

void S950FilterProcessor::setStateInformation (const void* d, int sz)
{ if (auto xml = getXmlFromBinary (d, sz)) apvts.replaceState (juce::ValueTree::fromXml (*xml)); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new S950FilterProcessor(); }
