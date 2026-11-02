#include "MMFilterView.h"
#include "MMFilterDSP.h"

namespace wvzz::modules {

MMFilterView::MMFilterView(juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& prefix,
                           ui::IHelpOverlayHost* helpHost)
    : apvtsState(apvts),
      instancePrefix(prefix),
      card("MM Filter", ui::WvZzLookAndFeel::neonGreen)
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
    initRotary(cutoffKnob,   paramId("cutoff"),    "Cutoff",    false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(resKnob,      paramId("res"),       "Resonance", false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(mixKnob,      paramId("mix"),       "Mix",       false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(envAttKnob,   paramId("env_att"),   "Env Att",   false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(envRelKnob,   paramId("env_rel"),   "Env Rel",   false, ui::WvZzLookAndFeel::neonGreen);
    initRotary(envDepthKnob, paramId("env_depth"), "Env Mod",   true,  ui::WvZzLookAndFeel::neonGreen);
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

    // Top combo box
    auto typeArea = content.removeFromTop(28);
    typeBox.label.setBounds(typeArea.removeFromLeft(65));
    typeArea.removeFromLeft(4);
    typeBox.box.setBounds(typeArea);

    content.removeFromTop(6);

    // Row 1: Cutoff, Res, Mix
    auto row1 = content.removeFromTop(68);
    const int colW1 = row1.getWidth() / 3;
    const int knobSize = 46;

    auto placeKnob = [](RotaryControl& rc, juce::Rectangle<int> colRect, int kSize) {
        int kx = colRect.getX() + (colRect.getWidth() - kSize) / 2;
        rc.slider.setBounds(kx, colRect.getY() + 2, kSize, kSize);
        rc.label.setBounds(colRect.getX(), colRect.getY() + kSize + 2, colRect.getWidth(), 14);
    };

    placeKnob(cutoffKnob, row1.removeFromLeft(colW1), knobSize);
    placeKnob(resKnob, row1.removeFromLeft(colW1), knobSize);
    placeKnob(mixKnob, row1, knobSize);

    content.removeFromTop(6);

    // Row 2: Env Att, Env Rel, Env Mod
    auto row2 = content.removeFromTop(68);
    const int colW2 = row2.getWidth() / 3;

    placeKnob(envAttKnob, row2.removeFromLeft(colW2), knobSize);
    placeKnob(envRelKnob, row2.removeFromLeft(colW2), knobSize);
    placeKnob(envDepthKnob, row2, knobSize);
}

} // namespace wvzz::modules
