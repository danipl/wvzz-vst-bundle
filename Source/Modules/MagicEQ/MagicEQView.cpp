#include "MagicEQView.h"

namespace wvzz::modules {

MagicEQView::MagicEQView(juce::AudioProcessorValueTreeState& apvts,
                         const juce::String& prefix,
                         ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("MAGIC EQ", ui::WvZzLookAndFeel::neonAmber)
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
    initRotary(amountKnob, paramId("amount"), "Amount", true, ui::WvZzLookAndFeel::neonAmber);
    initRotary(tuneKnob,   paramId("tune"),   "Tune",   false, ui::WvZzLookAndFeel::neonAmber);

    // Phat Mode Toggle Button
    phatToggle.button.setButtonText("PHAT");
    phatToggle.button.getProperties().set("isPill", true);
    phatToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::neonAmber.getARGB()));
    phatToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId("phat"), phatToggle.button);
    addAndMakeVisible(phatToggle.button);
}

void MagicEQView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText,
                            bool isBipolar, juce::Colour accent)
{
    rc.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rc.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rc.slider.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    if (isBipolar)
        rc.slider.getProperties().set("isBipolar", true);
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
    auto meqTop = content.removeFromTop(20);
    phatToggle.button.setBounds(meqTop.removeFromRight(50).reduced(0, 1));

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    layoutKnob(amountKnob, content.removeFromLeft(content.getWidth() / 2));
    layoutKnob(tuneKnob, content);
}

} // namespace wvzz::modules
