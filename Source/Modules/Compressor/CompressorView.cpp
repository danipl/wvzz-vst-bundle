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
      card("Compressor", ui::WvZzLookAndFeel::neonGreen)
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
    const int meterW = 20;
    const int meterH = content.getHeight() - 10;
    const int meterX = content.getX() + 6;
    const int meterY = content.getY() + 4;
    grMeter->setBounds(meterX, meterY, meterW, meterH);

    auto controlsArea = content.withTrimmedLeft(meterW + 12);
    const int colW = controlsArea.getWidth() / 3;
    const int knobSize = 46;
    const int ky = controlsArea.getY() + (controlsArea.getHeight() - (knobSize + 16)) / 2;

    auto placeKnob = [&](RotaryControl& rc, int col) {
        int cx = controlsArea.getX() + col * colW + (colW - knobSize) / 2;
        rc.slider.setBounds(cx, ky, knobSize, knobSize);
        rc.label.setBounds(controlsArea.getX() + col * colW, ky + knobSize + 2, colW, 14);
    };

    placeKnob(amountKnob, 0);
    placeKnob(releaseKnob, 1);

    const int btnW = 44;
    const int btnH = 22;
    const int bx = controlsArea.getX() + 2 * colW + (colW - btnW) / 2;
    const int by = controlsArea.getY() + (controlsArea.getHeight() - btnH) / 2;
    phatToggle.button.setBounds(bx, by, btnW, btnH);
}

} // namespace wvzz::modules
