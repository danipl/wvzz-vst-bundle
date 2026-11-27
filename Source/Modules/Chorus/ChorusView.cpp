#include "ChorusView.h"

namespace wvzz::modules {

ChorusView::ChorusView(juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& prefix,
                       ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("CHORUS", ui::WvZzLookAndFeel::skyCyan)
{
    addAndMakeVisible(card);

    auto paramId = [this](const juce::String& name) {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    };
    const auto accent = ui::WvZzLookAndFeel::skyCyan;

    // Header bypass
    ui::initToggle(*this, apvtsState, bypassToggle, paramId("bypass"), {}, accent, true);
    bypassToggle.button.onStateChange = [this] { card.setBypassed(bypassToggle.button.getToggleState()); };
    card.setBypassed(bypassToggle.button.getToggleState());

    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("chorus", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    ui::initRotary(*this, apvtsState, rateKnob,     paramId("rate"),     "Rate",     accent);
    ui::initRotary(*this, apvtsState, depthKnob,    paramId("depth"),    "Depth",    accent);
    ui::initRotary(*this, apvtsState, delayKnob,    paramId("delay"),    "Delay",    accent);
    ui::initRotary(*this, apvtsState, spreadKnob,   paramId("spread"),   "Spread",   accent);
    ui::initRotary(*this, apvtsState, feedbackKnob, paramId("feedback"), "Feedback", accent);
    ui::initRotary(*this, apvtsState, mixKnob,      paramId("mix"),      "Mix",      accent);
    ui::initCombo(*this, apvtsState, voicesBox, paramId("voices"), "Voices");
}

void ChorusView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();
    ui::layoutCombo(voicesBox, content.removeFromBottom(28).withTrimmedRight(content.getWidth() / 2));

    auto topRow = content.removeFromTop(content.getHeight() / 2);
    const int knobW = content.getWidth() / 3;
    ui::layoutKnob(rateKnob,  topRow.removeFromLeft(knobW));
    ui::layoutKnob(depthKnob, topRow.removeFromLeft(knobW));
    ui::layoutKnob(delayKnob, topRow);

    ui::layoutKnob(spreadKnob,   content.removeFromLeft(knobW));
    ui::layoutKnob(feedbackKnob, content.removeFromLeft(knobW));
    ui::layoutKnob(mixKnob,      content);
}

} // namespace wvzz::modules
