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

    void initRotary(RotaryControl& ctrl, const juce::String& paramId, const juce::String& labelText, bool isBipolar = false);
    void initToggle(ToggleControl& ctrl, const juce::String& paramId, const juce::String& text);
    void initCombo(ComboControl& ctrl, const juce::String& paramId, const juce::String& labelText, const juce::StringArray& items);

    CamelPhatAudioProcessor& audioProcessor;
    camelphat::ui::CamelPhatLookAndFeel lookAndFeel;
    camelphat::ui::HelpContentRepository helpRepo;

    // Content container for proportional 900x560 canvas scaling
    juce::Component contentContainer;

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

    // Single Central Assignable X/Y Pad
    camelphat::ui::XYPadComponent xyPad;
    ComboControl xyXTargetBox;
    ComboControl xyYTargetBox;
    RotaryControl xyXDepthKnob;
    RotaryControl xyYDepthKnob;

    // LFO 1 & LFO 2 Controls
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

    // Left Column: BP Filter
    ToggleControl bpBypassToggle;
    ToggleControl bpLinkToggle;
    juce::Slider bpLowCutSlider;
    juce::Slider bpHighCutSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpLowCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpHighCutAttachment;
    juce::Label bpLowCutLabel;
    juce::Label bpHighCutLabel;
    RotaryControl bpLowResKnob;
    RotaryControl bpHighResKnob;
    RotaryControl bpBrMixKnob;

    // Left Column: Distortion
    ToggleControl distBypassToggle;
    RotaryControl distMechKnob;
    RotaryControl distTubeKnob;
    RotaryControl distBitKnob;
    RotaryControl distXcitaKnob;

    // Right Column: MM Filter
    ToggleControl mmBypassToggle;
    ComboControl mmTypeBox;
    RotaryControl mmCutoffKnob;
    RotaryControl mmResKnob;
    RotaryControl mmMixKnob;
    RotaryControl mmEnvAttKnob;
    RotaryControl mmEnvRelKnob;
    RotaryControl mmEnvDepthKnob;

    // Right Column: Flanger
    ToggleControl flgBypassToggle;
    RotaryControl flgRateKnob;
    RotaryControl flgAmountKnob;

    // Right Column: Magic EQ
    ToggleControl meqBypassToggle;
    ToggleControl meqPhatToggle;
    RotaryControl meqAmountKnob;
    RotaryControl meqTuneKnob;

    // Right Column: Compressor
    ToggleControl compBypassToggle;
    ToggleControl compPhatToggle;
    RotaryControl compAmountKnob;
    RotaryControl compReleaseKnob;

    // Right Column: Master
    ToggleControl masterBypassToggle;
    RotaryControl inputGainKnob;
    RotaryControl masterMixKnob;
    RotaryControl masterVolKnob;
    juce::TextButton diceButton;
    camelphat::ui::StereoVUMeterComponent vuMeter;

    // Preset selector
    juce::ComboBox presetSelector;

    // Help Floating Bubble
    camelphat::ui::HelpOverlayComponent helpOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CamelPhatAudioProcessorEditor)
};
