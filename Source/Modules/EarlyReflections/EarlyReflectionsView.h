#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/ModuleViewControls.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class EarlyReflectionsView : public ui::IModuleView
{
public:
    EarlyReflectionsView(juce::AudioProcessorValueTreeState& apvts,
               const juce::String& prefix,
               ui::IHelpOverlayHost* helpHost);
    ~EarlyReflectionsView() override = default;

    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;
    ui::ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    ui::RotaryControl sizeKnob;
    ui::RotaryControl diffusionKnob;
    ui::RotaryControl levelKnob;
    juce::Label hintLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarlyReflectionsView)
};

} // namespace wvzz::modules
