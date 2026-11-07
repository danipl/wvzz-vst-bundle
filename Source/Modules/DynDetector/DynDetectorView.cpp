#include "DynDetectorView.h"
#include "DynDetectorDSP.h"

namespace wvzz::modules {

DynDetectorView::DynDetectorView(juce::AudioProcessorValueTreeState& apvts,
                                 const juce::String& prefix,
                                 const DynDetectorDSP* /*detDsp*/,
                                 ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("SUB & DETECTOR", ui::WvZzLookAndFeel::warmAmber)
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
        helpButton = std::make_unique<ui::HelpButtonComponent>("dyn_detector", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Toggles
    initToggle(subLockToggle, paramId("sub_lock"), "LOCK", true, ui::WvZzLookAndFeel::warmAmber);
    initToggle(scListenToggle, paramId("sc_listen"), "LISTEN", true, ui::WvZzLookAndFeel::coralRed);

    // Rotary Knobs
    initRotary(subXoverKnob, paramId("sub_xover"), "X-Over", false, ui::WvZzLookAndFeel::warmAmber);
    initRotary(subTightKnob, paramId("sub_tight"), "Tight",  false, ui::WvZzLookAndFeel::warmAmber);
    initRotary(scHpKnob,     paramId("sc_hp"),     "SC HPF", false, ui::WvZzLookAndFeel::warmAmber);
    initRotary(scTiltKnob,   paramId("sc_tilt"),   "Tilt",   true,  ui::WvZzLookAndFeel::warmAmber);
}

void DynDetectorView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText,
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

void DynDetectorView::initToggle(ToggleControl& tc, const juce::String& paramId, const juce::String& text,
                                 bool isPill, juce::Colour accent)
{
    tc.button.setButtonText(text);
    tc.button.getProperties().set("isPill", isPill);
    tc.button.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    tc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId, tc.button);
    addAndMakeVisible(tc.button);
}

void DynDetectorView::paint(juce::Graphics&)
{
}

void DynDetectorView::updateVisuals()
{
}

void DynDetectorView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    // Top control strip: Sub Lock and SC Listen pills
    auto topRow = content.removeFromTop(20);
    subLockToggle.button.setBounds(topRow.removeFromLeft(50).reduced(0, 1));
    scListenToggle.button.setBounds(topRow.removeFromRight(56).reduced(0, 1));

    content.removeFromTop(4);

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    const int kw = content.getWidth() / 4;
    layoutKnob(subXoverKnob, content.removeFromLeft(kw));
    layoutKnob(subTightKnob, content.removeFromLeft(kw));
    layoutKnob(scHpKnob,     content.removeFromLeft(kw));
    layoutKnob(scTiltKnob,   content);
}

} // namespace wvzz::modules
