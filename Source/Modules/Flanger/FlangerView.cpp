#include "FlangerView.h"

namespace wvzz::modules {

FlangerView::FlangerView(juce::AudioProcessorValueTreeState& apvts,
                         const juce::String& prefix,
                         ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("Flanger", ui::WvZzLookAndFeel::neonCyan)
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
        helpButton = std::make_unique<ui::HelpButtonComponent>("flanger", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Rotary Knobs: Rate, Amount
    initRotary(rateKnob,   paramId("rate"),   "Rate",   ui::WvZzLookAndFeel::neonCyan);
    initRotary(amountKnob, paramId("amount"), "Amount", ui::WvZzLookAndFeel::neonCyan);
}

void FlangerView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
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

void FlangerView::paint(juce::Graphics&)
{
}

void FlangerView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();
    const int colW = content.getWidth() / 2;
    const int knobSize = 46;
    const int ky = content.getY() + (content.getHeight() - (knobSize + 16)) / 2;

    auto placeKnob = [&](RotaryControl& rc, int col) {
        int cx = content.getX() + col * colW + (colW - knobSize) / 2;
        rc.slider.setBounds(cx, ky, knobSize, knobSize);
        rc.label.setBounds(content.getX() + col * colW, ky + knobSize + 2, colW, 14);
    };

    placeKnob(rateKnob, 0);
    placeKnob(amountKnob, 1);
}

} // namespace wvzz::modules
