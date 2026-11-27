#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/ModuleViewControls.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class GrainView : public ui::IModuleView
{
public:
    GrainView(juce::AudioProcessorValueTreeState& apvts,
               const juce::String& prefix,
               ui::IHelpOverlayHost* helpHost);
    ~GrainView() override = default;

    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    ui::RotaryControl sizeKnob;
    ui::RotaryControl densityKnob;
    ui::RotaryControl pitchKnob;
    ui::RotaryControl sprayKnob;
    ui::RotaryControl reverseKnob;
    ui::RotaryControl spreadKnob;
    juce::Label hintLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainView)
};

} // namespace wvzz::modules
