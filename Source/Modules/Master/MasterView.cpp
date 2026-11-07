#include "MasterView.h"

namespace wvzz::modules {

MasterView::MasterView(juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& prefix,
                       const IMeterSource* outMeterSource,
                       const IMeterSource* inMeterSource,
                       ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      outMeter(outMeterSource),
      inMeter(inMeterSource),
      card("MASTER", ui::WvZzLookAndFeel::arcticIce)
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

    // Header Help Button
    if (helpHost != nullptr)
    {
        helpButton = std::make_unique<ui::HelpButtonComponent>("master", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Rotary Knobs: In Gain, Dry/Wet, Volume
    initRotary(inputGainKnob, "input_gain", "In Gain", ui::WvZzLookAndFeel::arcticIce);
    initRotary(mixKnob, paramId("mix"), "Dry/Wet", ui::WvZzLookAndFeel::arcticIce);
    initRotary(volKnob, paramId("vol"), "Volume", ui::WvZzLookAndFeel::arcticIce);

    // Pill toggles: CLIP and MONO (<120Hz)
    clipToggle.button.setButtonText("CLIP");
    clipToggle.button.getProperties().set("isPill", true);
    clipToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::warmAmber.getARGB()));
    clipToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId("clip"), clipToggle.button);
    addAndMakeVisible(clipToggle.button);

    monoBassToggle.button.setButtonText("MONO");
    monoBassToggle.button.getProperties().set("isPill", true);
    monoBassToggle.button.getProperties().set("accentColor", static_cast<int>(ui::WvZzLookAndFeel::arcticIce.getARGB()));
    monoBassToggle.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsState, paramId("mono_bass"), monoBassToggle.button);
    addAndMakeVisible(monoBassToggle.button);

    // Stereo VU Meters
    if (inMeter != nullptr)
    {
        inputMeter = std::make_unique<ui::StereoVUMeterComponent>(*inMeter, "IN");
        addAndMakeVisible(*inputMeter);
    }
    if (outMeter != nullptr)
    {
        outputMeter = std::make_unique<ui::StereoVUMeterComponent>(*outMeter, "OUT");
        addAndMakeVisible(*outputMeter);
    }
}

void MasterView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
{
    rc.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rc.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rc.slider.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    rc.slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    addAndMakeVisible(rc.slider);

    rc.label.setText(labelText, juce::dontSendNotification);
    rc.label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    rc.label.setJustificationType(juce::Justification::centred);
    rc.label.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(rc.label);

    rc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsState, paramId, rc.slider);
}

void MasterView::paint(juce::Graphics&)
{
}

void MasterView::updateVisuals()
{
    if (inputMeter != nullptr)
        inputMeter->repaint();
    if (outputMeter != nullptr)
        outputMeter->repaint();
}

void MasterView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    auto vuArea = content.removeFromRight(56).reduced(1, 4);
    int meterW = (vuArea.getWidth() - 4) / 2;
    if (inputMeter != nullptr)
        inputMeter->setBounds(vuArea.removeFromLeft(meterW));
    vuArea.removeFromLeft(4);
    if (outputMeter != nullptr)
        outputMeter->setBounds(vuArea);

    auto topPillRow = content.removeFromTop(18);
    clipToggle.button.setBounds(topPillRow.removeFromRight(44).reduced(0, 1));
    topPillRow.removeFromRight(4);
    monoBassToggle.button.setBounds(topPillRow.removeFromRight(48).reduced(0, 1));

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    int mKw = content.getWidth() / 3;
    layoutKnob(inputGainKnob, content.removeFromLeft(mKw));
    layoutKnob(mixKnob, content.removeFromLeft(mKw));
    layoutKnob(volKnob, content);
}

} // namespace wvzz::modules
