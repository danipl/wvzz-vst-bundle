#include "PhaserView.h"

namespace wvzz::modules {

PhaserView::PhaserView(juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& prefix,
                       ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("PHASER", ui::WvZzLookAndFeel::emeraldGreen)
{
    addAndMakeVisible(card);

    auto paramId = [this](const juce::String& name) {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    };
    const auto accent = ui::WvZzLookAndFeel::emeraldGreen;

    // Header bypass
    ui::initToggle(*this, apvtsState, bypassToggle, paramId("bypass"), {}, accent, true);
    bypassToggle.button.onStateChange = [this] { card.setBypassed(bypassToggle.button.getToggleState()); };
    card.setBypassed(bypassToggle.button.getToggleState());

    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("phaser", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    ui::initRotary(*this, apvtsState, rateKnob,     paramId("rate"),     "Rate",     accent);
    ui::initRotary(*this, apvtsState, depthKnob,    paramId("depth"),    "Depth",    accent);
    ui::initRotary(*this, apvtsState, centerKnob,   paramId("center"),   "Center",   accent);
    ui::initRotary(*this, apvtsState, feedbackKnob, paramId("feedback"), "Feedback", accent, true);
    ui::initRotary(*this, apvtsState, stereoKnob,   paramId("stereo"),   "Stereo",   accent);
    ui::initRotary(*this, apvtsState, mixKnob,      paramId("mix"),      "Mix",      accent);
    ui::initCombo(*this, apvtsState, stagesBox, paramId("stages"), "Stages");
}

void PhaserView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();
    ui::layoutCombo(stagesBox, content.removeFromBottom(28).withTrimmedRight(content.getWidth() / 2));

    auto topRow = content.removeFromTop(content.getHeight() / 2);
    const int knobW = content.getWidth() / 3;
    ui::layoutKnob(rateKnob,   topRow.removeFromLeft(knobW));
    ui::layoutKnob(depthKnob,  topRow.removeFromLeft(knobW));
    ui::layoutKnob(centerKnob, topRow);

    ui::layoutKnob(feedbackKnob, content.removeFromLeft(knobW));
    ui::layoutKnob(stereoKnob,   content.removeFromLeft(knobW));
    ui::layoutKnob(mixKnob,      content);
}

} // namespace wvzz::modules
