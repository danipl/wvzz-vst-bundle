#include "MultibandOTTView.h"
#include "MultibandOTTDSP.h"

namespace wvzz::modules {

MultibandOTTView::MultibandOTTView(juce::AudioProcessorValueTreeState& apvts,
                                   const juce::String& prefix,
                                   const MultibandOTTDSP* /*ottDsp*/,
                                   ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("HYPER-OTT (3-BAND)", ui::WvZzLookAndFeel::skyCyan)
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

    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("multiband_ott", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // 5 Knobs
    initRotary(depthKnob,     paramId("depth"),      "Depth",     ui::WvZzLookAndFeel::skyCyan);
    initRotary(upDriveKnob,   paramId("up_drive"),   "Upward",    ui::WvZzLookAndFeel::skyCyan);
    initRotary(downDriveKnob, paramId("down_drive"), "Downward",  ui::WvZzLookAndFeel::skyCyan);
    initRotary(timeKnob,      paramId("time"),       "Time",      ui::WvZzLookAndFeel::skyCyan);
    initRotary(sheenKnob,     paramId("sheen"),      "Sheen",     ui::WvZzLookAndFeel::coralRed);
}

void MultibandOTTView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
{
    rc.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rc.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rc.slider.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    rc.slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    addAndMakeVisible(rc.slider);

    rc.label.setText(labelText, juce::dontSendNotification);
    rc.label.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    rc.label.setJustificationType(juce::Justification::centred);
    rc.label.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(rc.label);

    rc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsState, paramId, rc.slider);
}

void MultibandOTTView::paint(juce::Graphics&)
{
}

void MultibandOTTView::updateVisuals()
{
}

void MultibandOTTView::resized()
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

    const int kw = content.getWidth() / 5;
    layoutKnob(depthKnob,     content.removeFromLeft(kw));
    layoutKnob(upDriveKnob,   content.removeFromLeft(kw));
    layoutKnob(downDriveKnob, content.removeFromLeft(kw));
    layoutKnob(timeKnob,      content.removeFromLeft(kw));
    layoutKnob(sheenKnob,     content);
}

} // namespace wvzz::modules
