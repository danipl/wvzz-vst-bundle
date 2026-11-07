#include "DistortionView.h"

namespace wvzz::modules {

DistortionView::DistortionView(juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& prefix,
                               ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("DISTORTION", ui::WvZzLookAndFeel::coralRed)
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
        helpButton = std::make_unique<ui::HelpButtonComponent>("distortion", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Mode Selector (Classic / Clip / Fold)
    modeSelector.box.addItem("Classic", 1);
    modeSelector.box.addItem("Hard Clip", 2);
    modeSelector.box.addItem("Wavefold", 3);
    modeSelector.box.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modeSelector.box);
    modeSelector.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsState, paramId("mode"), modeSelector.box);

    // Rotary Knobs: Mech, Tube, Bitcrush, Xcita
    initRotary(mechKnob, paramId("mech"), "Mech", ui::WvZzLookAndFeel::coralRed);
    initRotary(tubeKnob, paramId("tube"), "Tube", ui::WvZzLookAndFeel::coralRed);
    initRotary(bitKnob, paramId("bit"), "Bitcrush", ui::WvZzLookAndFeel::coralRed);
    initRotary(xcitaKnob, paramId("xcita"), "Xcita", ui::WvZzLookAndFeel::coralRed);
}

void DistortionView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
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

void DistortionView::paint(juce::Graphics&)
{
}

void DistortionView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    auto modeArea = content.removeFromTop(20);
    modeSelector.box.setBounds(modeArea.reduced(24, 0));
    content.removeFromTop(2);

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(4, 2);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    // 2x2 grid for generous knob sizing and balanced vertical spacing
    const int rowHeight = content.getHeight() / 2;
    auto topRow = content.removeFromTop(rowHeight);
    auto bottomRow = content;

    const int colWidth = topRow.getWidth() / 2;
    layoutKnob(mechKnob, topRow.removeFromLeft(colWidth));
    layoutKnob(tubeKnob, topRow);

    layoutKnob(bitKnob, bottomRow.removeFromLeft(colWidth));
    layoutKnob(xcitaKnob, bottomRow);
}

} // namespace wvzz::modules
