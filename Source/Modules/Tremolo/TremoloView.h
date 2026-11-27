#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/ModuleViewControls.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class TremoloView : public ui::IModuleView
{
public:
    TremoloView(juce::AudioProcessorValueTreeState& apvts,
               const juce::String& prefix,
               ui::IHelpOverlayHost* helpHost);
    ~TremoloView() override = default;

    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;
    ui::ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    ui::RotaryControl rateKnob;
    ui::RotaryControl depthKnob;
    ui::RotaryControl stereoKnob;
    ui::RotaryControl xoverKnob;
    ui::ComboControl shapeBox;
    ui::ComboControl modeBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TremoloView)
};

} // namespace wvzz::modules
