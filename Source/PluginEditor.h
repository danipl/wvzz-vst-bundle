#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/CamelPhatLookAndFeel.h"
#include "UI/ModuleCardComponent.h"
#include "UI/IHelpSystem.h"
#include "UI/HelpContentRepository.h"
#include "UI/HelpButtonComponent.h"
#include "UI/HelpOverlayComponent.h"
#include "UI/XYPadComponent.h"
#include "UI/StereoVUMeterComponent.h"
#include <vector>
#include <memory>

class CamelPhatAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      public camelphat::ui::IHelpOverlayHost,
                                      private juce::Timer
{
public:
    explicit CamelPhatAudioProcessorEditor(CamelPhatAudioProcessor& proc);
    ~CamelPhatAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    // IHelpOverlayHost interface implementation
    void showHelp(const juce::String& topicId, juce::Rectangle<int> targetScreenBounds) override;
    void hideHelp() override;
    [[nodiscard]] bool isHelpVisible() const noexcept override;

private:
    void timerCallback() override;

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

    // Sub-component: BP Filter Passband visualizer between vertical sliders
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

    // Sub-component: Live LFO Waveform Preview Box
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

    // Sub-component: Compressor Gain Reduction Meter
    class CompressorGRMeter : public juce::Component
    {
    public:
        explicit CompressorGRMeter(const camelphat::dsp::CompressorModule& comp)
            : compressor(comp) {}

        void paint(juce::Graphics& g) override;
    private:
        const camelphat::dsp::CompressorModule& compressor;
    };

    // Responsive Canvas Component that paints the dark background and top header bar
    class ContentContainer : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
    };

    void initRotary(RotaryControl& ctrl, const juce::String& paramId, const juce::String& labelText,
                    bool isBipolar = false, juce::Colour accent = camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    void initToggle(ToggleControl& ctrl, const juce::String& paramId, const juce::String& text,
                    bool isHeaderBypass = false, bool isPill = false,
                    juce::Colour accent = camelphat::ui::CamelPhatLookAndFeel::neonGreen);
    void initCombo(ComboControl& ctrl, const juce::String& paramId, const juce::String& labelText,
                   const juce::StringArray& items);

    void updateModuleBypassVisuals();

    CamelPhatAudioProcessor& audioProcessor;
    camelphat::ui::CamelPhatLookAndFeel lookAndFeel;
    camelphat::ui::HelpContentRepository helpRepo;

    // Content container for proportional 1080x680 canvas scaling
    ContentContainer contentContainer;

    // Top Header Bar Controls
    juce::Label titleLabel;
    juce::Label subTitleLabel;
    juce::TextButton prevPresetButton;
    juce::ComboBox presetSelector;
    juce::TextButton nextPresetButton;
    juce::TextButton diceButton;
    ToggleControl masterBypassToggle;

    // Cards
    camelphat::ui::ModuleCardComponent bpFilterCard;
    camelphat::ui::ModuleCardComponent distCard;
    camelphat::ui::ModuleCardComponent centerModCard;
    camelphat::ui::ModuleCardComponent mmFilterCard;
    camelphat::ui::ModuleCardComponent flangerCard;
    camelphat::ui::ModuleCardComponent magicEqCard;
    camelphat::ui::ModuleCardComponent compCard;
    camelphat::ui::ModuleCardComponent masterCard;

    // Help Buttons
    camelphat::ui::HelpButtonComponent bpHelp;
    camelphat::ui::HelpButtonComponent distHelp;
    camelphat::ui::HelpButtonComponent centerModHelp;
    camelphat::ui::HelpButtonComponent mmHelp;
    camelphat::ui::HelpButtonComponent flangerHelp;
    camelphat::ui::HelpButtonComponent eqHelp;
    camelphat::ui::HelpButtonComponent compHelp;
    camelphat::ui::HelpButtonComponent masterHelp;

    // Header-Integrated Module Bypass Buttons
    ToggleControl bpBypassToggle;
    ToggleControl distBypassToggle;
    ToggleControl mmBypassToggle;
    ToggleControl flgBypassToggle;
    ToggleControl meqBypassToggle;
    ToggleControl compBypassToggle;

    // Left Column: BP Filter
    ToggleControl bpLinkToggle;
    juce::Slider bpLowCutSlider;
    juce::Slider bpHighCutSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpLowCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpHighCutAttachment;
    juce::Label bpLowCutLabel;
    juce::Label bpHighCutLabel;
    std::unique_ptr<PassbandVisualizer> bpPassband;
    RotaryControl bpLowResKnob;
    RotaryControl bpHighResKnob;
    RotaryControl bpBrMixKnob;

    // Left Column: Distortion
    RotaryControl distMechKnob;
    RotaryControl distTubeKnob;
    RotaryControl distBitKnob;
    RotaryControl distXcitaKnob;

    // Center Column: Assignable X/Y Pad & Controls
    camelphat::ui::XYPadComponent xyPad;
    ComboControl xyXTargetBox;
    ComboControl xyYTargetBox;
    RotaryControl xyXDepthKnob;
    RotaryControl xyYDepthKnob;

    // Center Column: LFO 1 & LFO 2 Sub-panels
    std::unique_ptr<LFOScopeComponent> lfo1Scope;
    std::unique_ptr<LFOScopeComponent> lfo2Scope;
    ComboControl lfo1WaveBox;
    ComboControl lfo1TargetBox;
    ComboControl lfo1SyncDivBox;
    ToggleControl lfo1SyncToggle;
    RotaryControl lfo1RateKnob;
    RotaryControl lfo1DepthKnob;

    ComboControl lfo2WaveBox;
    ComboControl lfo2TargetBox;
    ComboControl lfo2SyncDivBox;
    ToggleControl lfo2SyncToggle;
    RotaryControl lfo2RateKnob;
    RotaryControl lfo2DepthKnob;

    // Right Column: MM Filter
    ComboControl mmTypeBox;
    RotaryControl mmCutoffKnob;
    RotaryControl mmResKnob;
    RotaryControl mmMixKnob;
    RotaryControl mmEnvAttKnob;
    RotaryControl mmEnvRelKnob;
    RotaryControl mmEnvDepthKnob;

    // Right Column: Flanger
    RotaryControl flgRateKnob;
    RotaryControl flgAmountKnob;

    // Right Column: Magic EQ
    ToggleControl meqPhatToggle;
    RotaryControl meqAmountKnob;
    RotaryControl meqTuneKnob;

    // Right Column: Compressor
    ToggleControl compPhatToggle;
    RotaryControl compAmountKnob;
    RotaryControl compReleaseKnob;
    std::unique_ptr<CompressorGRMeter> compMeter;

    // Right Column: Master
    RotaryControl inputGainKnob;
    RotaryControl masterMixKnob;
    RotaryControl masterVolKnob;
    camelphat::ui::StereoVUMeterComponent vuMeter;

    // Help Floating Bubble
    camelphat::ui::HelpOverlayComponent helpOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CamelPhatAudioProcessorEditor)
};
