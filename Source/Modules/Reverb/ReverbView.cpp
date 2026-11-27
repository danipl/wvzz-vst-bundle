#include "ReverbView.h"

namespace wvzz::modules {

ReverbView::ReverbView(juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& prefix,
                       ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("REVERB", ui::WvZzLookAndFeel::skyCyan)
{
    addAndMakeVisible(card);

    auto paramId = [this](const juce::String& name) {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    };
    const auto accent = ui::WvZzLookAndFeel::skyCyan;

    // Header bypass (follows clicks, presets, and host automation)
    ui::initToggle(*this, apvtsState, bypassToggle, paramId("bypass"), {}, accent, true);
    bypassToggle.button.onStateChange = [this] { card.setBypassed(bypassToggle.button.getToggleState()); };
    card.setBypassed(bypassToggle.button.getToggleState());

    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("reverb", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    const auto toneAccent = ui::WvZzLookAndFeel::emeraldGreen;
    const auto shimmerAccent = ui::WvZzLookAndFeel::electricViolet;

    ui::initCombo(*this, apvtsState, modeBox, paramId("mode"), "Mode");
    ui::initCombo(*this, apvtsState, intervalBox, paramId("interval"), "Interval");

    // Row 1: space
    ui::initRotary(*this, apvtsState, predelayKnob, paramId("predelay"), "Pre-Delay", accent);
    ui::initRotary(*this, apvtsState, sizeKnob,     paramId("size"),     "Size",      accent);
    ui::initRotary(*this, apvtsState, decayKnob,    paramId("decay"),    "Decay",     accent);
    ui::initRotary(*this, apvtsState, mixKnob,      paramId("mix"),      "Mix",       accent);

    // Row 2: tone and movement
    ui::initRotary(*this, apvtsState, lodampKnob,    paramId("lodamp"),    "Low Damp",  toneAccent);
    ui::initRotary(*this, apvtsState, hidampKnob,    paramId("hidamp"),    "High Damp", toneAccent);
    ui::initRotary(*this, apvtsState, diffusionKnob, paramId("diffusion"), "Diffusion", toneAccent);
    ui::initRotary(*this, apvtsState, modKnob,       paramId("mod"),       "Tail Mod",  toneAccent);

    // Row 3: stereo and shimmer
    ui::initRotary(*this, apvtsState, widthKnob,   paramId("width"),   "Width",   accent);
    ui::initRotary(*this, apvtsState, shimmerKnob, paramId("shimmer"), "Shimmer", shimmerAccent);

    modeHintLabel.setFont(juce::FontOptions(10.0f));
    modeHintLabel.setJustificationType(juce::Justification::centredLeft);
    modeHintLabel.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(modeHintLabel);

    updateVisuals();
}

void ReverbView::updateVisuals()
{
    // The shimmer controls act only in Shimmer mode
    const int mode = modeBox.box.getSelectedItemIndex();
    const bool shimmerMode = (mode == 2);
    shimmerKnob.slider.setAlpha(shimmerMode ? 1.0f : 0.35f);
    intervalBox.box.setAlpha(shimmerMode ? 1.0f : 0.35f);

    static const char* hints[] = {
        "Plate: short delay lines. Dense and bright.",
        "Hall: long delay lines. Builds up slowly.",
        "Shimmer: Hall with a pitch-shifted tail in the loop."
    };
    const juce::String hint = hints[juce::jlimit(0, 2, mode)];
    if (modeHintLabel.getText() != hint)
        modeHintLabel.setText(hint, juce::dontSendNotification);
}

void ReverbView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();
    auto row = content.removeFromTop(28);
    ui::layoutCombo(modeBox, row.removeFromLeft(row.getWidth() / 2), 40);
    ui::layoutCombo(intervalBox, row, 52);

    content.removeFromTop(2);
    modeHintLabel.setBounds(content.removeFromTop(18));
    content.removeFromTop(4);

    const int rowH = content.getHeight() / 3;
    const int knobW = content.getWidth() / 4;

    auto r1 = content.removeFromTop(rowH);
    ui::layoutKnob(predelayKnob, r1.removeFromLeft(knobW));
    ui::layoutKnob(sizeKnob,     r1.removeFromLeft(knobW));
    ui::layoutKnob(decayKnob,    r1.removeFromLeft(knobW));
    ui::layoutKnob(mixKnob,      r1);

    auto r2 = content.removeFromTop(rowH);
    ui::layoutKnob(lodampKnob,    r2.removeFromLeft(knobW));
    ui::layoutKnob(hidampKnob,    r2.removeFromLeft(knobW));
    ui::layoutKnob(diffusionKnob, r2.removeFromLeft(knobW));
    ui::layoutKnob(modKnob,       r2);

    auto r3 = content;
    ui::layoutKnob(widthKnob,   r3.removeFromLeft(knobW));
    ui::layoutKnob(shimmerKnob, r3.removeFromLeft(knobW));
}

} // namespace wvzz::modules
