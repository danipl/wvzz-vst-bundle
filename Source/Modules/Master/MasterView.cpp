#include "MasterView.h"

namespace wvzz::modules {

MasterView::MasterView(juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& prefix,
                       const IMeterSource* meterSource,
                       ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      meter(meterSource),
      card("Master", ui::WvZzLookAndFeel::neonAmber)
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
        helpButton = std::make_unique<ui::HelpButtonComponent>("master", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Rotary Knobs: Mix, Volume
    initRotary(mixKnob, paramId("mix"), "Mix", ui::WvZzLookAndFeel::neonAmber);
    initRotary(volKnob, paramId("vol"), "Volume", ui::WvZzLookAndFeel::neonAmber);

    // Stereo VU Meter
    if (meter != nullptr)
    {
        vuMeter = std::make_unique<ui::StereoVUMeterComponent>(*meter, "OUT");
        addAndMakeVisible(*vuMeter);
    }
}

void MasterView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
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

void MasterView::paint(juce::Graphics&)
{
}

void MasterView::updateVisuals()
{
    if (vuMeter != nullptr)
        vuMeter->repaint();
}

void MasterView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();
    const int meterW = 30;
    const int meterH = content.getHeight() - 8;
    const int meterX = content.getX() + 6;
    const int meterY = content.getY() + 4;
    if (vuMeter != nullptr)
        vuMeter->setBounds(meterX, meterY, meterW, meterH);

    auto controlsArea = content.withTrimmedLeft(meterW + 12);
    const int colW = controlsArea.getWidth() / 2;
    const int knobSize = 46;
    const int ky = controlsArea.getY() + (controlsArea.getHeight() - (knobSize + 16)) / 2;

    auto placeKnob = [&](RotaryControl& rc, int col) {
        int cx = controlsArea.getX() + col * colW + (colW - knobSize) / 2;
        rc.slider.setBounds(cx, ky, knobSize, knobSize);
        rc.label.setBounds(controlsArea.getX() + col * colW, ky + knobSize + 2, colW, 14);
    };

    placeKnob(mixKnob, 0);
    placeKnob(volKnob, 1);
}

} // namespace wvzz::modules
