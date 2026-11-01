#include "PluginEditor.h"

static const juce::Colour kPanel  (0xffc9c6bd);  // façade beige/gris du S950
static const juce::Colour kDark   (0xff2b2b2b);
static const juce::Colour kRed    (0xffd8342a);

void S950Look::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                 float a0, float a1, juce::Slider&)
{
    auto r = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (14.0f);
    const float d = juce::jmin (r.getWidth(), r.getHeight());
    auto b = juce::Rectangle<float> (d, d).withCentre (r.getCentre());
    const float ang = a0 + pos * (a1 - a0);

    // graduations 0..9
    for (int i = 0; i <= 10; ++i)
    {
        const float a = a0 + (float) i / 10.0f * (a1 - a0);
        const auto c = b.getCentre();
        const float r1 = d * 0.5f + 4.0f, r2 = d * 0.5f + (i % 5 == 0 ? 12.0f : 8.0f);
        g.setColour (kDark);
        g.drawLine (c.x + std::sin (a) * r1, c.y - std::cos (a) * r1,
                    c.x + std::sin (a) * r2, c.y - std::cos (a) * r2, 1.5f);
    }
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (b.translated (2.0f, 3.0f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a4a4a), b.getX(), b.getY(),
                                             juce::Colour (0xff151515), b.getRight(), b.getBottom(), false));
    g.fillEllipse (b);
    g.setColour (juce::Colours::black);
    g.drawEllipse (b, 1.5f);
    auto inner = b.reduced (d * 0.12f);
    g.setColour (juce::Colour (0xff1d1d1d));
    g.fillEllipse (inner);

    juce::Path p;
    p.addRoundedRectangle (-2.5f, -d * 0.44f, 5.0f, d * 0.26f, 1.5f);
    g.setColour (kRed);
    g.fillPath (p, juce::AffineTransform::rotation (ang).translated (b.getCentreX(), b.getCentreY()));
}

S950FilterEditor::S950FilterEditor (S950FilterProcessor& p)
    : AudioProcessorEditor (&p), proc (p), att (p.apvts, "filter", knob)
{
    knob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    knob.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
    knob.setLookAndFeel (&look);
    knob.setDoubleClickReturnValue (true, 99.0);
    addAndMakeVisible (knob);
    setSize (360, 300);
    startTimerHz (20);
}

S950FilterEditor::~S950FilterEditor() { knob.setLookAndFeel (nullptr); }

void S950FilterEditor::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (kPanel.brighter (0.1f), 0, 0, kPanel.darker (0.15f), 0, area.getBottom(), false));
    g.fillAll();
    g.setColour (kDark); g.drawRect (area, 3.0f);

    // bandeau titre (texte original : le logo Akai est une marque déposée, non reproduit)
    g.setColour (kDark);
    g.fillRect (juce::Rectangle<float> (0, 0, area.getWidth(), 44.0f));
    g.setColour (kRed);
    g.setFont (juce::FontOptions ("Arial", 22.0f, juce::Font::bold));
    g.drawText ("S950  FILTER", 16, 0, 200, 44, juce::Justification::centredLeft);
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("12 BIT  6th ORDER LOW PASS", 160, 0, 190, 44, juce::Justification::centredRight);

    // afficheur rouge 0..99
    auto disp = juce::Rectangle<float> (area.getCentreX() - 40.0f, 54.0f, 80.0f, 34.0f);
    g.setColour (juce::Colours::black); g.fillRoundedRectangle (disp, 3.0f);
    g.setColour (kRed);
    g.setFont (juce::FontOptions ("Courier New", 28.0f, juce::Font::bold));
    g.drawText (juce::String ((int) knob.getValue()).paddedLeft ('0', 2), disp.toNearestInt(), juce::Justification::centred);

    g.setColour (kDark);
    g.setFont (juce::FontOptions ("Arial", 13.0f, juce::Font::bold));
    g.drawText ("FILTER", 0, getHeight() - 26, getWidth(), 20, juce::Justification::centred);
}

void S950FilterEditor::resized() { knob.setBounds (80, 94, 200, 180); }
