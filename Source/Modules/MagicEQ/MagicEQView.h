#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class MagicEQView : public ui::IModuleView
{
public:
    MagicEQView(juce::AudioProcessorValueTreeState& apvts,
                const juce::String& prefix,
                ui::IHelpOverlayHost* helpHost);
    ~MagicEQView() override = default;

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
                    bool isBipolar = false, juce::Colour accent = ui::WvZzLookAndFeel::neonAmber);

    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;

    ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    RotaryControl amountKnob;
    RotaryControl tuneKnob;
    ToggleControl phatToggle;
    ToggleControl preToggle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MagicEQView)
};

} // namespace wvzz::modules
