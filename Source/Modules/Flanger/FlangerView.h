#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class FlangerView : public ui::IModuleView
{
public:
    FlangerView(juce::AudioProcessorValueTreeState& apvts,
                const juce::String& prefix,
                ui::IHelpOverlayHost* helpHost);
    ~FlangerView() override = default;

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
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

    void initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText,
                    juce::Colour accent = ui::WvZzLookAndFeel::neonCyan);

    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;

    ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    RotaryControl rateKnob;
    RotaryControl amountKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlangerView)
};

} // namespace wvzz::modules
