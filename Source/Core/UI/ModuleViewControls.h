#pragma once

#include "WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

namespace wvzz::ui {

/**
 * @brief Shared control bundles and helpers for module views (knob + label + attachment, etc.).
 * Keeps the look of the existing cards: bold 10.5 pt labels under rotary knobs, pill toggles.
 */
struct RotaryControl
{
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

struct ToggleControl
{
    juce::ToggleButton button;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
};

struct ComboControl
{
    juce::ComboBox box;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

inline void initRotary(juce::Component& parent, juce::AudioProcessorValueTreeState& apvts, RotaryControl& rc,
                       const juce::String& paramId, const juce::String& labelText,
                       juce::Colour accent, bool isBipolar = false)
{
    rc.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rc.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rc.slider.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    if (isBipolar)
        rc.slider.getProperties().set("isBipolar", true);
    rc.slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    parent.addAndMakeVisible(rc.slider);

    rc.label.setText(labelText, juce::dontSendNotification);
    rc.label.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    rc.label.setJustificationType(juce::Justification::centred);
    rc.label.setColour(juce::Label::textColourId, WvZzLookAndFeel::textSecondary);
    parent.addAndMakeVisible(rc.label);

    rc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, rc.slider);
}

inline void initCombo(juce::Component& parent, juce::AudioProcessorValueTreeState& apvts, ComboControl& cc,
                      const juce::String& paramId, const juce::String& labelText)
{
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramId)))
    {
        for (int i = 0; i < choice->choices.size(); ++i)
            cc.box.addItem(choice->choices[i], i + 1);
    }

    cc.label.setText(labelText, juce::dontSendNotification);
    cc.label.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    cc.label.setJustificationType(juce::Justification::centredLeft);
    cc.label.setColour(juce::Label::textColourId, WvZzLookAndFeel::textSecondary);
    parent.addAndMakeVisible(cc.label);
    parent.addAndMakeVisible(cc.box);

    cc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, paramId, cc.box);
}

inline void initToggle(juce::Component& parent, juce::AudioProcessorValueTreeState& apvts, ToggleControl& tc,
                       const juce::String& paramId, const juce::String& text,
                       juce::Colour accent, bool isHeaderBypass = false)
{
    tc.button.setButtonText(text);
    tc.button.getProperties().set(isHeaderBypass ? "isHeaderBypass" : "isPill", true);
    tc.button.getProperties().set("accentColor", static_cast<int>(accent.getARGB()));
    tc.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, paramId, tc.button);
    parent.addAndMakeVisible(tc.button);
}

inline void layoutKnob(RotaryControl& rc, juce::Rectangle<int> area)
{
    auto knobArea = area.reduced(1);
    rc.label.setBounds(knobArea.removeFromBottom(14));
    rc.slider.setBounds(knobArea);
}

/** Label on the left (labelWidth px), combo box on the right. */
inline void layoutCombo(ComboControl& cc, juce::Rectangle<int> area, int labelWidth = 52)
{
    cc.label.setBounds(area.removeFromLeft(labelWidth));
    cc.box.setBounds(area.reduced(2, 3));
}

} // namespace wvzz::ui
