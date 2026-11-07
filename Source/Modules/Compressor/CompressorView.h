#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class CompressorDSP;

class CompressorView : public ui::IModuleView
{
public:
    CompressorView(juce::AudioProcessorValueTreeState& apvts,
                   const juce::String& prefix,
                   const CompressorDSP* compDsp,
                   ui::IHelpOverlayHost* helpHost);
    ~CompressorView() override = default;

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

    class GRMeterComponent : public juce::Component
    {
    public:
        explicit GRMeterComponent(const IMeterSource* source, const CompressorDSP* dsp = nullptr)
            : meterSource(source), compDsp(dsp) {}

        void updateLevels();
        void paint(juce::Graphics& g) override;

    private:
        const IMeterSource* meterSource { nullptr };
        const CompressorDSP* compDsp   { nullptr };
        float currentGrNorm { 0.0f };
        float peakHoldNorm  { 0.0f };
        float displayGrDb   { 0.0f };
        bool  isSaturating  { false };
    };

    void initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText,
                    juce::Colour accent = ui::WvZzLookAndFeel::honeyGold);

    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;
    const CompressorDSP* compressorDSP;

    ui::ModuleCardComponent card;

    ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    RotaryControl threshKnob;
    RotaryControl ratioKnob;
    RotaryControl attackKnob;
    RotaryControl releaseKnob;
    RotaryControl mixKnob;
    ToggleControl phatToggle;

    std::unique_ptr<GRMeterComponent> grMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorView)
};

} // namespace wvzz::modules
