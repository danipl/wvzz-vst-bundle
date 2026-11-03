#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/XYPadComponent.h"
#include "../../Core/UI/SignalComparisonComponent.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include "../../Core/UI/IHelpSystem.h"
#include "ModulationDSP.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class ModulationView : public ui::IModuleView
{
public:
    ModulationView(juce::AudioProcessorValueTreeState& apvts,
                   ModulationDSP& dsp,
                   dsp::AudioVisualizerSource* visualizerSource,
                   ui::IHelpOverlayHost* helpHost);
    ~ModulationView() override = default;

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

    struct ComboControl
    {
        juce::ComboBox box;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    class LFOScopeComponent : public juce::Component
    {
    public:
        LFOScopeComponent(std::function<int()> getWave,
                          std::function<float()> getPhase,
                          std::function<float()> getSnap,
                          juce::Colour scopeColour)
            : waveFn(std::move(getWave)),
              phaseFn(std::move(getPhase)),
              snapFn(std::move(getSnap)),
              colour(scopeColour) {}

        void paint(juce::Graphics& g) override;

    private:
        std::function<int()> waveFn;
        std::function<float()> phaseFn;
        std::function<float()> snapFn;
        juce::Colour colour;
    };

    void initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText,
                    bool isBipolar = false, juce::Colour accent = ui::WvZzLookAndFeel::neonGreen);
    void initCombo(ComboControl& cc, const juce::String& paramId, const juce::String& labelText,
                   const juce::StringArray& items);
    void initToggle(ToggleControl& tc, const juce::String& paramId, const juce::String& text,
                    bool isPill = true, juce::Colour accent = ui::WvZzLookAndFeel::neonGreen);

    void setCenterTab(bool showMonitor);

    juce::AudioProcessorValueTreeState& apvtsState;
    ModulationDSP& modDSP;

    // Card container
    ui::ModuleCardComponent card;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    // Center Tab Bar in header
    juce::TextButton xyTabButton { "X/Y PAD" };
    juce::TextButton monitorTabButton { "MONITOR" };
    bool isMonitorActive { true };

    // Upper views
    std::unique_ptr<ui::XYPadComponent> xyPad;
    std::unique_ptr<ui::SignalComparisonComponent> signalVisualizer;

    ComboControl xyXTargetBox;
    ComboControl xyYTargetBox;
    RotaryControl xyXDepthKnob;
    RotaryControl xyYDepthKnob;

    // LFO 1 Panel
    std::unique_ptr<LFOScopeComponent> lfo1Scope;
    ComboControl lfo1WaveBox;
    ToggleControl lfo1SyncToggle;
    ComboControl lfo1SyncDivBox;
    ComboControl lfo1TargetBox;
    RotaryControl lfo1RateKnob;
    RotaryControl lfo1DepthKnob;

    // LFO 2 Panel
    std::unique_ptr<LFOScopeComponent> lfo2Scope;
    ComboControl lfo2WaveBox;
    ToggleControl lfo2SyncToggle;
    ComboControl lfo2SyncDivBox;
    ComboControl lfo2TargetBox;
    RotaryControl lfo2RateKnob;
    RotaryControl lfo2DepthKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationView)
};

} // namespace wvzz::modules
