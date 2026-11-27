#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/ModuleViewControls.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class ReverbView : public ui::IModuleView
{
public:
    ReverbView(juce::AudioProcessorValueTreeState& apvts,
               const juce::String& prefix,
               ui::IHelpOverlayHost* helpHost);
    ~ReverbView() override = default;

    void resized() override;
    void updateVisuals() override;

private:
    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;
    ui::ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    ui::ComboControl modeBox;
    ui::ComboControl intervalBox;

    ui::RotaryControl predelayKnob;
    ui::RotaryControl sizeKnob;
    ui::RotaryControl decayKnob;
    ui::RotaryControl mixKnob;
    ui::RotaryControl lodampKnob;
    ui::RotaryControl hidampKnob;
    ui::RotaryControl diffusionKnob;
    ui::RotaryControl modKnob;
    ui::RotaryControl widthKnob;
    ui::RotaryControl shimmerKnob;

    juce::Label modeHintLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbView)
};

} // namespace wvzz::modules
