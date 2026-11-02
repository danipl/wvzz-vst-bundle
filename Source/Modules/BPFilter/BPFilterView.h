#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class BPFilterView : public ui::IModuleView
{
public:
    BPFilterView(juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& prefix,
                 ui::IHelpOverlayHost* helpHost);
    ~BPFilterView() override = default;

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

    class PassbandVisualizer : public juce::Component
    {
    public:
        PassbandVisualizer(juce::Slider& low, juce::Slider& high)
            : lowSlider(low), highSlider(high) {}

        void paint(juce::Graphics& g) override;

    private:
        juce::Slider& lowSlider;
        juce::Slider& highSlider;
    };

    void initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText,
                    juce::Colour accent = ui::WvZzLookAndFeel::neonCyan);

    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;

    // Header Controls
    ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    // Faders & Passband
    juce::Slider lowCutSlider;
    juce::Slider highCutSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highCutAttachment;
    juce::Label lowCutLabel;
    juce::Label highCutLabel;
    std::unique_ptr<PassbandVisualizer> passbandVisualizer;
    ToggleControl linkToggle;

    // Knobs
    RotaryControl lowResKnob;
    RotaryControl highResKnob;
    RotaryControl brMixKnob;

    bool isUpdatingCutoffs { false };
    double lastLowCutVal { 20.0 };
    double lastHighCutVal { 20000.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BPFilterView)
};

} // namespace wvzz::modules
