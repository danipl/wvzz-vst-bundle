#include "DistortionView.h"

namespace wvzz::modules {

DistortionView::DistortionView(juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& prefix,
                               ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("Distortion", ui::WvZzLookAndFeel::neonAmber)
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

    // Rotary Knobs: Mech, Tube, Bitcrush, Xcita
    initRotary(mechKnob, paramId("mech"), "Mech", ui::WvZzLookAndFeel::neonAmber);
    initRotary(tubeKnob, paramId("tube"), "Tube", ui::WvZzLookAndFeel::neonAmber);
    initRotary(bitKnob, paramId("bit"), "Bitcrush", ui::WvZzLookAndFeel::neonAmber);
    initRotary(xcitaKnob, paramId("xcita"), "Xcita", ui::WvZzLookAndFeel::neonAmber);
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

    // 2x2 grid for 4 knobs
    const int rowHeight = content.getHeight() / 2;
    const int colWidth = content.getWidth() / 2;
    const int knobSize = 48;

    auto placeKnob = [&](RotaryControl& rc, int col, int row) {
        int cellX = content.getX() + col * colWidth;
        int cellY = content.getY() + row * rowHeight;
        int kx = cellX + (colWidth - knobSize) / 2;
        int ky = cellY + 6;
        rc.slider.setBounds(kx, ky, knobSize, knobSize);
        rc.label.setBounds(cellX, ky + knobSize + 2, colWidth, 14);
    };

    placeKnob(mechKnob, 0, 0);
    placeKnob(tubeKnob, 1, 0);
    placeKnob(bitKnob, 0, 1);
    placeKnob(xcitaKnob, 1, 1);
}

} // namespace wvzz::modules
