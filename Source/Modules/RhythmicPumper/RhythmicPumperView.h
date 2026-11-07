#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class RhythmicPumperDSP;

class RhythmicPumperView : public ui::IModuleView
{
public:
    RhythmicPumperView(juce::AudioProcessorValueTreeState& apvts,
                       const juce::String& prefix,
                       const RhythmicPumperDSP* pumpDsp,
                       ui::IHelpOverlayHost* helpHost);
    ~RhythmicPumperView() override = default;

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
                    juce::Colour accent = ui::WvZzLookAndFeel::electricViolet);
    void initToggle(ToggleControl& tc, const juce::String& paramId, const juce::String& text,
                    bool isPill = true, juce::Colour accent = ui::WvZzLookAndFeel::electricViolet);

    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;

    ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    ToggleControl subOnlyToggle;
    juce::ComboBox modeSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;

    juce::ComboBox syncDivSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> syncDivAttachment;

    juce::ComboBox shapeSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shapeAttachment;

    RotaryControl amountKnob;
    RotaryControl curveKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RhythmicPumperView)
};

} // namespace wvzz::modules
