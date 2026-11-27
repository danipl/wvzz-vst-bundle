#include "FlangerView.h"

namespace wvzz::modules {

FlangerView::FlangerView(juce::AudioProcessorValueTreeState& apvts,
                         const juce::String& prefix,
                         ui::IHelpOverlayHost* helpHost,
                         bool extendedControls)
    : apvtsState(apvts),
      instancePrefix(prefix),
      extended(extendedControls),
      card("FLANGER", ui::WvZzLookAndFeel::electricViolet)
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
    initRotary(rateKnob,   paramId("rate"),   "Rate",   ui::WvZzLookAndFeel::electricViolet);
    initRotary(amountKnob, paramId("amount"), "Amount", ui::WvZzLookAndFeel::electricViolet);

    if (extended)
    {
        initRotary(feedbackKnob, paramId("feedback"), "Feedback", ui::WvZzLookAndFeel::electricViolet);
        feedbackKnob.slider.getProperties().set("isBipolar", true);
        initRotary(depthKnob,  paramId("depth"),  "Depth",  ui::WvZzLookAndFeel::electricViolet);
        initRotary(centerKnob, paramId("center"), "Center", ui::WvZzLookAndFeel::electricViolet);

        tzeroToggle.button.setButtonText("THRU-ZERO");
        tzeroToggle.button.getProperties().set("isPill", true);
        tzeroToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::electricViolet.getARGB()));
        tzeroToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvtsState, paramId("tzero"), tzeroToggle.button);
        addAndMakeVisible(tzeroToggle.button);

        // Follow preset and host changes too, not only clicks
        bypassToggle.button.onStateChange = [this] { card.setBypassed(bypassToggle.button.getToggleState()); };
    }
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

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    if (extended)
    {
        // Row 1: Rate, Amount, Feedback. Row 2: Depth, Center, Through-Zero toggle.
        auto topRow = content.removeFromTop(content.getHeight() / 2);
        const int knobW = content.getWidth() / 3;
        layoutKnob(rateKnob,     topRow.removeFromLeft(knobW));
        layoutKnob(amountKnob,   topRow.removeFromLeft(knobW));
        layoutKnob(feedbackKnob, topRow);

        layoutKnob(depthKnob,  content.removeFromLeft(knobW));
        layoutKnob(centerKnob, content.removeFromLeft(knobW));
        tzeroToggle.button.setBounds(content.withSizeKeepingCentre(juce::jmin(92, content.getWidth() - 6), 22));
        return;
    }

    layoutKnob(rateKnob, content.removeFromLeft(content.getWidth() / 2));
    layoutKnob(amountKnob, content);
}

} // namespace wvzz::modules
