#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class DynDetectorDSP;

class DynDetectorView : public ui::IModuleView
{
public:
    DynDetectorView(juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& prefix,
                    const DynDetectorDSP* detDsp,
                    ui::IHelpOverlayHost* helpHost);
    ~DynDetectorView() override = default;

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
                    bool isBipolar = false, juce::Colour accent = ui::WvZzLookAndFeel::warmAmber);
    void initToggle(ToggleControl& tc, const juce::String& paramId, const juce::String& text,
                    bool isPill = true, juce::Colour accent = ui::WvZzLookAndFeel::warmAmber);

    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;

    ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    ToggleControl subLockToggle;
    ToggleControl scListenToggle;

    RotaryControl subXoverKnob;
    RotaryControl subTightKnob;
    RotaryControl scHpKnob;
    RotaryControl scTiltKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynDetectorView)
};

} // namespace wvzz::modules
