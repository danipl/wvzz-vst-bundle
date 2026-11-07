#include "MaximizerClipperView.h"
#include "MaximizerClipperDSP.h"

namespace wvzz::modules {

MaximizerClipperView::MaximizerClipperView(juce::AudioProcessorValueTreeState& apvts,
                                           const juce::String& prefix,
                                           const MaximizerClipperDSP* /*clipDsp*/,
                                           ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("MAXIMIZER CLIPPER", ui::WvZzLookAndFeel::coralRed)
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
        helpButton = std::make_unique<ui::HelpButtonComponent>("maximizer_clipper", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Mode Selector
    const auto& modes = getClipperModeNames();
    for (int i = 0; i < modes.size(); ++i)
        modeSelector.addItem(modes[i], i + 1);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsState, paramId("mode"), modeSelector);
    addAndMakeVisible(modeSelector);

    // Knobs
    initRotary(driveKnob,   paramId("drive"),   "Drive",   ui::WvZzLookAndFeel::coralRed);
    initRotary(ceilingKnob, paramId("ceiling"), "Ceiling", ui::WvZzLookAndFeel::coralRed);
    initRotary(mixKnob,     paramId("mix"),     "Mix",     ui::WvZzLookAndFeel::coralRed);
}

void MaximizerClipperView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
{
    rc.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rc.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rc.slider.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
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

void MaximizerClipperView::paint(juce::Graphics&)
{
}

void MaximizerClipperView::updateVisuals()
{
}

void MaximizerClipperView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    // Top Row: Mode ComboBox right aligned
    auto topRow = content.removeFromTop(20);
    modeSelector.setBounds(topRow.removeFromRight(100).reduced(0, 1));

    content.removeFromTop(4);

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    const int kw = content.getWidth() / 3;
    layoutKnob(driveKnob,   content.removeFromLeft(kw));
    layoutKnob(ceilingKnob, content.removeFromLeft(kw));
    layoutKnob(mixKnob,     content);
}

} // namespace wvzz::modules
