#include "GrainView.h"

namespace wvzz::modules {

GrainView::GrainView(juce::AudioProcessorValueTreeState& apvts,
                     const juce::String& prefix,
                     ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("GRAIN", ui::WvZzLookAndFeel::electricViolet)
{
    addAndMakeVisible(card);

    auto paramId = [this](const juce::String& name) {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    };
    const auto accent = ui::WvZzLookAndFeel::electricViolet;

    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("grain", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    ui::initRotary(*this, apvtsState, sizeKnob,    paramId("size"),    "Size",    accent);
    ui::initRotary(*this, apvtsState, densityKnob, paramId("density"), "Density", accent);
    ui::initRotary(*this, apvtsState, pitchKnob,   paramId("pitch"),   "Pitch",   accent, true);
    ui::initRotary(*this, apvtsState, sprayKnob,   paramId("spray"),   "Spray",   accent);
    ui::initRotary(*this, apvtsState, reverseKnob, paramId("reverse"), "Reverse", accent);
    ui::initRotary(*this, apvtsState, spreadKnob,  paramId("spread"),  "Spread",  accent);

    hintLabel.setText("Used when the Delay mode is Granular. 32 voices.", juce::dontSendNotification);
    hintLabel.setFont(juce::FontOptions(10.0f));
    hintLabel.setJustificationType(juce::Justification::centred);
    hintLabel.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(hintLabel);
}

void GrainView::resized()
{
    card.setBounds(getLocalBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();
    hintLabel.setBounds(content.removeFromBottom(18));

    auto topRow = content.removeFromTop(content.getHeight() / 2);
    const int knobW = content.getWidth() / 3;
    ui::layoutKnob(sizeKnob,    topRow.removeFromLeft(knobW));
    ui::layoutKnob(densityKnob, topRow.removeFromLeft(knobW));
    ui::layoutKnob(pitchKnob,   topRow);

    ui::layoutKnob(sprayKnob,   content.removeFromLeft(knobW));
    ui::layoutKnob(reverseKnob, content.removeFromLeft(knobW));
    ui::layoutKnob(spreadKnob,  content);
}

} // namespace wvzz::modules
