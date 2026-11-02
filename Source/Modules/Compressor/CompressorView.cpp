#include "CompressorView.h"

namespace wvzz::modules {

void CompressorView::GRMeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBg.darker(0.3f));
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(ui::WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    float grDb = 0.0f;
    if (meterSource != nullptr)
        grDb = meterSource->getGainReduction(); // Negative or 0.0f

    // Scale: 0 dB at top to -24 dB at bottom
    const float norm = juce::jlimit(0.0f, 1.0f, -grDb / 24.0f);

    if (norm > 0.005f)
    {
        const float meterH = bounds.getHeight() * norm;
        auto meterRect = juce::Rectangle<float>(bounds.getX() + 1.0f, bounds.getY() + 1.0f,
                                                bounds.getWidth() - 2.0f, meterH);
        juce::ColourGradient grGrad(ui::WvZzLookAndFeel::neonAmber,
                                    bounds.getX(), bounds.getY(),
                                    juce::Colour(0xffff1744),
                                    bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(grGrad);
        g.fillRoundedRectangle(meterRect, 2.0f);
    }

    // Label: GR
    g.setColour(ui::WvZzLookAndFeel::textSecondary);
    g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
    g.drawText("GR", bounds.removeFromBottom(12.0f), juce::Justification::centred, false);
}

CompressorView::CompressorView(juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& prefix,
                               const IMeterSource* meterSource,
                               ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      meter(meterSource),
      card("COMPRESSOR", ui::WvZzLookAndFeel::neonGreen)
{
    addAndMakeVisible(card);

    auto paramId = [this](const juce::String& name) {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    };

    // Header Bypass
    bypassToggle.button.getProperties().set("isHeaderBypass", true);
    bypassToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId("bypass"), bypassToggle.button);
    bypassToggle.button.onClick = [this] {
        card.setBypassed(bypassToggle.button.getToggleState());
    };
    addAndMakeVisible(bypassToggle.button);
    card.setBypassed(bypassToggle.button.getToggleState());

    // Header Help Button
    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("compressor", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Rotary Knobs: Amount, Release
    initRotary(amountKnob,  paramId("amount"),  "Amount",  ui::WvZzLookAndFeel::neonGreen);
    initRotary(releaseKnob, paramId("release"), "Release", ui::WvZzLookAndFeel::neonGreen);

    // Phat Mode Toggle
    phatToggle.button.setButtonText("PHAT");
    phatToggle.button.getProperties().set("isPill", true);
    phatToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::neonGreen.getARGB()));
    phatToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId("phat"), phatToggle.button);
    addAndMakeVisible(phatToggle.button);

    // GR Meter
    grMeter = std::make_unique<GRMeterComponent>(meter);
    addAndMakeVisible(*grMeter);
}

void CompressorView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
{
    rc.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rc.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rc.slider.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    rc.slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    addAndMakeVisible(rc.slider);

    rc.label.setText(labelText, juce::dontSendNotification);
    rc.label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    rc.label.setJustificationType(juce::Justification::centred);
    rc.label.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(rc.label);

    rc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsState, paramId, rc.slider);
}

void CompressorView::paint(juce::Graphics&)
{
}

void CompressorView::updateVisuals()
{
    if (grMeter != nullptr)
        grMeter->repaint();
}

void CompressorView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    auto compTop = content.removeFromTop(20);
    phatToggle.button.setBounds(compTop.removeFromRight(50).reduced(0, 1));

    if (grMeter != nullptr)
        grMeter->setBounds(content.removeFromRight(32).reduced(2, 4));

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    int compKw = content.getWidth() / 2;
    layoutKnob(amountKnob, content.removeFromLeft(compKw));
    layoutKnob(releaseKnob, content);
}

} // namespace wvzz::modules
