#include "DelayView.h"

namespace wvzz::modules {

DelayView::DelayView(juce::AudioProcessorValueTreeState& apvts,
                     const juce::String& prefix,
                     ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("DELAY", ui::WvZzLookAndFeel::skyCyan)
{
    addAndMakeVisible(card);

    auto paramId = [this](const juce::String& name) {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    };
    const auto accent = ui::WvZzLookAndFeel::skyCyan;
    const auto loopAccent = ui::WvZzLookAndFeel::emeraldGreen;
    const auto tapeAccent = ui::WvZzLookAndFeel::warmAmber;

    // Header bypass (follows clicks, presets, and host automation)
    ui::initToggle(*this, apvtsState, bypassToggle, paramId("bypass"), {}, accent, true);
    bypassToggle.button.onStateChange = [this] { card.setBypassed(bypassToggle.button.getToggleState()); };
    card.setBypassed(bypassToggle.button.getToggleState());

    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("delay", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    ui::initCombo(*this, apvtsState, modeBox, paramId("mode"), "Mode");
    ui::initCombo(*this, apvtsState, routingBox, paramId("routing"), "Routing");
    ui::initToggle(*this, apvtsState, syncToggle, paramId("sync"), "SYNC", accent);
    ui::initCombo(*this, apvtsState, syncDivBox, paramId("sync_div"), "Note");

    // Row 1: time and level
    ui::initRotary(*this, apvtsState, timeKnob,     paramId("time"),     "Time",     accent);
    ui::initRotary(*this, apvtsState, offsetKnob,   paramId("offset"),   "R Offset", accent);
    ui::initRotary(*this, apvtsState, feedbackKnob, paramId("feedback"), "Feedback", accent);
    ui::initRotary(*this, apvtsState, mixKnob,      paramId("mix"),      "Mix",      accent);

    // Row 2: feedback loop
    ui::initRotary(*this, apvtsState, hpKnob,        paramId("hp"),        "High-Pass", loopAccent);
    ui::initRotary(*this, apvtsState, lpKnob,        paramId("lp"),        "Low-Pass",  loopAccent);
    ui::initRotary(*this, apvtsState, diffusionKnob, paramId("diffusion"), "Diffusion", loopAccent);
    ui::initRotary(*this, apvtsState, duckKnob,      paramId("duck"),      "Ducking",   loopAccent);

    // Row 3: tape / BBD character
    ui::initRotary(*this, apvtsState, wowKnob,   paramId("wow"),   "Wow",   tapeAccent);
    ui::initRotary(*this, apvtsState, driveKnob, paramId("drive"), "Drive", tapeAccent);

    modeHintLabel.setFont(juce::FontOptions(10.0f));
    modeHintLabel.setJustificationType(juce::Justification::centredLeft);
    modeHintLabel.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(modeHintLabel);

    updateVisuals();
}

void DelayView::updateVisuals()
{
    // Dim the controls that the current settings do not use
    const int mode = modeBox.box.getSelectedItemIndex();
    const bool sync = syncToggle.button.getToggleState();
    const bool tapeLike = (mode == 1 || mode == 2);

    timeKnob.slider.setAlpha(sync ? 0.35f : 1.0f);
    syncDivBox.box.setAlpha(sync ? 1.0f : 0.35f);
    wowKnob.slider.setAlpha(tapeLike ? 1.0f : 0.35f);
    driveKnob.slider.setAlpha(mode == 1 ? 1.0f : 0.35f);

    static const char* hints[] = {
        "Clean: exact repeats. Time changes crossfade.",
        "Tape: Drive saturates, Wow bends the pitch.",
        "BBD: dark analog repeats. Wow at half depth.",
        "Granular: repeats built from grains (see GRAIN)."
    };
    const juce::String hint = hints[juce::jlimit(0, 3, mode)];
    if (modeHintLabel.getText() != hint)
        modeHintLabel.setText(hint, juce::dontSendNotification);
}

void DelayView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    // Top: mode + routing, then sync + note value
    auto row = content.removeFromTop(28);
    ui::layoutCombo(modeBox, row.removeFromLeft(row.getWidth() / 2), 40);
    ui::layoutCombo(routingBox, row, 50);

    content.removeFromTop(4);
    row = content.removeFromTop(28);
    syncToggle.button.setBounds(row.removeFromLeft(64).reduced(2, 3));
    row.removeFromLeft(8);
    ui::layoutCombo(syncDivBox, row.removeFromLeft(130), 36);

    content.removeFromTop(2);
    modeHintLabel.setBounds(content.removeFromTop(18));
    content.removeFromTop(4);

    // Three knob rows of equal height
    const int rowH = content.getHeight() / 3;
    const int knobW = content.getWidth() / 4;

    auto r1 = content.removeFromTop(rowH);
    ui::layoutKnob(timeKnob,     r1.removeFromLeft(knobW));
    ui::layoutKnob(offsetKnob,   r1.removeFromLeft(knobW));
    ui::layoutKnob(feedbackKnob, r1.removeFromLeft(knobW));
    ui::layoutKnob(mixKnob,      r1);

    auto r2 = content.removeFromTop(rowH);
    ui::layoutKnob(hpKnob,        r2.removeFromLeft(knobW));
    ui::layoutKnob(lpKnob,        r2.removeFromLeft(knobW));
    ui::layoutKnob(diffusionKnob, r2.removeFromLeft(knobW));
    ui::layoutKnob(duckKnob,      r2);

    auto r3 = content;
    ui::layoutKnob(wowKnob,   r3.removeFromLeft(knobW));
    ui::layoutKnob(driveKnob, r3.removeFromLeft(knobW));
}

} // namespace wvzz::modules
