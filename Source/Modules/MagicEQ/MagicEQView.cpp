#include "MagicEQView.h"

namespace wvzz::modules {

MagicEQView::MagicEQView(juce::AudioProcessorValueTreeState& apvts,
                         const juce::String& prefix,
                         ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("Magic EQ", ui::WvZzLookAndFeel::neonAmber)
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
        helpButton = std::make_unique<ui::HelpButtonComponent>("magic_eq", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Rotary Knobs: Amount, Tune
    initRotary(amountKnob, paramId("amount"), "Amount", ui::WvZzLookAndFeel::neonAmber);
    initRotary(tuneKnob,   paramId("tune"),   "Tune",   ui::WvZzLookAndFeel::neonAmber);

    // Phat Mode Toggle Button
    phatToggle.button.setButtonText("PHAT");
    phatToggle.button.getProperties().set("isPill", true);
    phatToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::neonAmber.getARGB()));
    phatToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId("phat"), phatToggle.button);
    addAndMakeVisible(phatToggle.button);
}

void MagicEQView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
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

void MagicEQView::paint(juce::Graphics&)
{
}

void MagicEQView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();
    const int colW = content.getWidth() / 3;
    const int knobSize = 46;
    const int ky = content.getY() + (content.getHeight() - (knobSize + 16)) / 2;

    auto placeKnob = [&](RotaryControl& rc, int col) {
        int cx = content.getX() + col * colW + (colW - knobSize) / 2;
        rc.slider.setBounds(cx, ky, knobSize, knobSize);
        rc.label.setBounds(content.getX() + col * colW, ky + knobSize + 2, colW, 14);
    };

    placeKnob(amountKnob, 0);
    placeKnob(tuneKnob, 1);

    // Place PHAT button in column 2
    const int btnW = 44;
    const int btnH = 22;
    const int bx = content.getX() + 2 * colW + (colW - btnW) / 2;
    const int by = content.getY() + (content.getHeight() - btnH) / 2;
    phatToggle.button.setBounds(bx, by, btnW, btnH);
}

} // namespace wvzz::modules
