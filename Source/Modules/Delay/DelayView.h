#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/ModuleViewControls.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class DelayView : public ui::IModuleView
{
public:
    DelayView(juce::AudioProcessorValueTreeState& apvts,
               const juce::String& prefix,
               ui::IHelpOverlayHost* helpHost);
    ~DelayView() override = default;

    void resized() override;
    void updateVisuals() override;

private:
    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    ui::ToggleControl bypassToggle;
    ui::ComboControl modeBox;
    ui::ComboControl routingBox;
    ui::ToggleControl syncToggle;
    ui::ComboControl syncDivBox;

    ui::RotaryControl timeKnob;
    ui::RotaryControl offsetKnob;
    ui::RotaryControl feedbackKnob;
    ui::RotaryControl mixKnob;
    ui::RotaryControl hpKnob;
    ui::RotaryControl lpKnob;
    ui::RotaryControl diffusionKnob;
    ui::RotaryControl duckKnob;
    ui::RotaryControl wowKnob;
    ui::RotaryControl driveKnob;

    juce::Label modeHintLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayView)
};

} // namespace wvzz::modules
