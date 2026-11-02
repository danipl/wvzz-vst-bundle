#include "MMFilterView.h"
#include "MMFilterDSP.h"

namespace wvzz::modules {

MMFilterView::MMFilterView(juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& prefix,
                           ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("MM FILTER", ui::WvZzLookAndFeel::neonGreen)
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
        helpButton = std::make_unique<ui::HelpButtonComponent>("mm_filter", *helpHost);
        addAndMakeVisible(*helpButton);
    }

    // Combo Box for Filter Type
    const auto& typeNames = getMMFilterTypeNames();
    for (int i = 0; i < typeNames.size(); ++i)
        typeBox.box.addItem(typeNames[i], i + 1);

    typeBox.label.setText("Filter Type", juce::dontSendNotification);
    typeBox.label.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    typeBox.label.setJustificationType(juce::Justification::centredLeft);
    typeBox.label.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(typeBox.label);
    addAndMakeVisible(typeBox.box);

    typeBox.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsState, paramId("type"), typeBox.box);

    // Rotary Knobs
    initRotary(cutoffKnob,   paramId("cutoff"),    "Cutoff",  false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(resKnob,      paramId("res"),       "Res",     false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(mixKnob,      paramId("mix"),       "Mix",     false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(envAttKnob,   paramId("env_att"),   "Attack",  false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(envRelKnob,   paramId("env_rel"),   "Release", false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(envDepthKnob, paramId("env_depth"), "Env Mod", true,  ui::WvZzLookAndFeel::neonGreen);
}

void MMFilterView::initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText,
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
    rc.label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    rc.label.setJustificationType(juce::Justification::centred);
    rc.label.setColour(juce::Label::textColourId, ui::WvZzLookAndFeel::textSecondary);
    addAndMakeVisible(rc.label);

    rc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvtsState, paramId, rc.slider);
}

void MMFilterView::paint(juce::Graphics&)
{
}

void MMFilterView::resized()
{
    card.setBounds(getLocalBounds());
    bypassToggle.button.setBounds(card.getHeaderBypassBounds());
    if (helpButton != nullptr)
        helpButton->setBounds(card.getHeaderHelpBounds());

    auto content = card.getContentBounds();

    auto mmTop = content.removeFromTop(26);
    typeBox.label.setBounds(mmTop.removeFromLeft(70));
    typeBox.box.setBounds(mmTop.removeFromLeft(170).reduced(2, 2));

    auto layoutKnob = [](RotaryControl& rc, juce::Rectangle<int> r) {
        int labelH = 14;
        auto knobArea = r.reduced(1);
        rc.label.setBounds(knobArea.removeFromBottom(labelH));
        rc.slider.setBounds(knobArea);
    };

    auto mmRow1 = content.removeFromTop(62);
    int mmKw1 = mmRow1.getWidth() / 4;
    layoutKnob(cutoffKnob, mmRow1.removeFromLeft(mmKw1));
    layoutKnob(resKnob, mmRow1.removeFromLeft(mmKw1));
    layoutKnob(mixKnob, mmRow1.removeFromLeft(mmKw1));
    layoutKnob(envDepthKnob, mmRow1);

    auto mmRow2 = content;
    int mmKw2 = mmRow2.getWidth() / 2;
    layoutKnob(envAttKnob, mmRow2.removeFromLeft(mmKw2));
    layoutKnob(envRelKnob, mmRow2);
}

} // namespace wvzz::modules
