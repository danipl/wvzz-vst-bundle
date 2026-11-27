#include "EarlyReflectionsView.h"

namespace wvzz::modules {

EarlyReflectionsView::EarlyReflectionsView(juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& prefix,
                       ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("EARLY REFLECTIONS", ui::WvZzLookAndFeel::emeraldGreen)
{
    addAndMakeVisible(card);

    auto paramId = [this](const juce::String& name) {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    };
    const auto accent = ui::WvZzLookAndFeel::emeraldGreen;

    // Header bypass (follows clicks, presets, and host automation)
    ui::initToggle(*this, apvtsState, bypassToggle, paramId("bypass"), {}, accent, true);
    bypassToggle.button.onStateChange = [this] { card.setBypassed(bypassToggle.button.getToggleState()); };
    card.setBypassed(bypassToggle.button.getToggleState());

    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("early_reflections", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    ui::initRotary(*this, apvtsState, sizeKnob,      paramId("size"),      "Size",      accent);
    ui::initRotary(*this, apvtsState, diffusionKnob, paramId("diffusion"), "Diffusion", accent);
    ui::initRotary(*this, apvtsState, levelKnob,     paramId("level"),     "Level",     accent);

    hintLabel.setText("First echoes of the room. They also feed the reverb tail.", juce::dontSendNotification);
    hintLabel.setFont(juce::FontOptions(10.0f));
    hintLabel.setJustificationType(juce::Justification::centred);
    hintLabel.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(hintLabel);
}

void EarlyReflectionsView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();
    hintLabel.setBounds(content.removeFromBottom(18));

    // Two knobs on top, the level knob centred below
    auto topRow = content.removeFromTop(content.getHeight() / 2);
    ui::layoutKnob(sizeKnob,      topRow.removeFromLeft(topRow.getWidth() / 2));
    ui::layoutKnob(diffusionKnob, topRow);
    ui::layoutKnob(levelKnob,     content.withSizeKeepingCentre(content.getWidth() / 2, content.getHeight()));
}

} // namespace wvzz::modules
