#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/StereoVUMeterComponent.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include "MasterDSP.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class MasterView : public ui::IModuleView
{
public:
    MasterView(juce::AudioProcessorValueTreeState& apvts,
               const juce::String& prefix,
               const IMeterSource* outMeterSource,
               const IMeterSource* inMeterSource,
               ui::IHelpOverlayHost* helpHost);
    ~MasterView() override = default;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void updateVisuals() override;

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
                    juce::Colour accent = ui::WvZzLookAndFeel::neonGreen);

    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;
    const IMeterSource* outMeter;
    const IMeterSource* inMeter;

    ui::ModuleCardComponent card;

    ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    RotaryControl inputGainKnob;
    RotaryControl mixKnob;
    RotaryControl volKnob;

    std::unique_ptr<ui::StereoVUMeterComponent> inputMeter;
    std::unique_ptr<ui::StereoVUMeterComponent> outputMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterView)
};

} // namespace wvzz::modules
