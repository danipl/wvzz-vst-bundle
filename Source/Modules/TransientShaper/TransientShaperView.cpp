#include "TransientShaperView.h"
#include "TransientShaperDSP.h"

namespace wvzz::modules {

TransientShaperView::TransientShaperView(juce::AudioProcessorValueTreeState& apvts,
                                         const juce::String& prefix,
                                         const TransientShaperDSP* /*transDsp*/,
                                         ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("TRANSIENT & GATE", ui::WvZzLookAndFeel::emeraldGreen)
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
        helpButton = std::make_unique<ui::HelpButtonComponent>("transient_shaper", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Speed Selector (Fast / Punch)
    speedSelector.addItem("Fast", 1);
    speedSelector.addItem("Punch", 2);
    speedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsState, paramId("speed"), speedSelector);
    addAndMakeVisible(speedSelector);

    // Knobs
    initRotary(attackKnob,  paramId("attack"),  "Attack",  true,  ui::WvZzLookAndFeel::emeraldGreen);
    initRotary(sustainKnob, paramId("sustain"), "Sustain", true,  ui::WvZzLookAndFeel::emeraldGreen);
    initRotary(gateKnob,    paramId("gate"),    "Gate",    false, ui::WvZzLookAndFeel::emeraldGreen);
}

void TransientShaperView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText,
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
    rc.label.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    rc.label.setJustificationType(juce::Justification::centred);
    rc.label.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(rc.label);

    rc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsState, paramId, rc.slider);
}

void TransientShaperView::paint(juce::Graphics&)
{
}

void TransientShaperView::updateVisuals()
{
}

void TransientShaperView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    // Top Row: Speed ComboBox right aligned
    auto topRow = content.removeFromTop(20);
    speedSelector.setBounds(topRow.removeFromRight(64).reduced(0, 1));

    content.removeFromTop(4);

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    const int kw = content.getWidth() / 3;
    layoutKnob(attackKnob,  content.removeFromLeft(kw));
    layoutKnob(sustainKnob, content.removeFromLeft(kw));
    layoutKnob(gateKnob,    content);
}

} // namespace wvzz::modules
