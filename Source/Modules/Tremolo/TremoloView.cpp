#include "TremoloView.h"

namespace wvzz::modules {

TremoloView::TremoloView(juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& prefix,
                       ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("TREMOLO", ui::WvZzLookAndFeel::warmAmber)
{
    addAndMakeVisible(card);

    auto paramId = [this](const juce::String& name) {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    };
    const auto accent = ui::WvZzLookAndFeel::warmAmber;

    // Header bypass
    ui::initToggle(*this, apvtsState, bypassToggle, paramId("bypass"), {}, accent, true);
    bypassToggle.button.onStateChange = [this] { card.setBypassed(bypassToggle.button.getToggleState()); };
    card.setBypassed(bypassToggle.button.getToggleState());

    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("tremolo", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    ui::initRotary(*this, apvtsState, rateKnob,   paramId("rate"),   "Rate",   accent);
    ui::initRotary(*this, apvtsState, depthKnob,  paramId("depth"),  "Depth",  accent);
    ui::initRotary(*this, apvtsState, stereoKnob, paramId("stereo"), "Stereo", accent);
    ui::initRotary(*this, apvtsState, xoverKnob,  paramId("xover"),  "X-Over", accent);
    ui::initCombo(*this, apvtsState, shapeBox, paramId("shape"), "Shape");
    ui::initCombo(*this, apvtsState, modeBox,  paramId("mode"),  "Mode");
}

void TremoloView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();
    auto comboRow = content.removeFromBottom(28);
    ui::layoutCombo(shapeBox, comboRow.removeFromLeft(comboRow.getWidth() / 2), 42);
    ui::layoutCombo(modeBox, comboRow, 42);

    const int knobW = content.getWidth() / 4;
    ui::layoutKnob(rateKnob,   content.removeFromLeft(knobW));
    ui::layoutKnob(depthKnob,  content.removeFromLeft(knobW));
    ui::layoutKnob(stereoKnob, content.removeFromLeft(knobW));
    ui::layoutKnob(xoverKnob,  content);
}

} // namespace wvzz::modules
