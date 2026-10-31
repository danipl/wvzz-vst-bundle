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
                                      public camelphat::ui::IHelpOverlayHost
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
    struct RotaryControl
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void initRotary(RotaryControl& ctrl, const juce::String& paramId, const juce::String& labelText);

    CamelPhatAudioProcessor& audioProcessor;
    camelphat::ui::CamelPhatLookAndFeel lookAndFeel;
    camelphat::ui::HelpContentRepository helpRepo;

    // Cards
    camelphat::ui::ModuleCardComponent bpFilterCard;
    camelphat::ui::ModuleCardComponent distCard;
    camelphat::ui::ModuleCardComponent mmFilterCard;
    camelphat::ui::ModuleCardComponent flangerCard;
    camelphat::ui::ModuleCardComponent magicEqCard;
    camelphat::ui::ModuleCardComponent compCard;
    camelphat::ui::ModuleCardComponent masterCard;

    // Help Buttons
    camelphat::ui::HelpButtonComponent bpHelp;
    camelphat::ui::HelpButtonComponent distHelp;
    camelphat::ui::HelpButtonComponent mmHelp;
    camelphat::ui::HelpButtonComponent flangerHelp;
    camelphat::ui::HelpButtonComponent eqHelp;
    camelphat::ui::HelpButtonComponent compHelp;
    camelphat::ui::HelpButtonComponent masterHelp;
    camelphat::ui::HelpButtonComponent xyFilterHelp;
    camelphat::ui::HelpButtonComponent xyDistHelp;

    // XY Pads
    camelphat::ui::XYPadComponent filterXYPad;
    camelphat::ui::XYPadComponent distXYPad;

    // Meters
    camelphat::ui::StereoVUMeterComponent vuMeter;

    // Rotary Knobs & Attachments
    // 1. BP Filter
    RotaryControl bpCutLow;
    RotaryControl bpCutHigh;
    RotaryControl bpRes;
    RotaryControl bpMix;
    RotaryControl bpRejectMix;

    // 2. Distortion
    RotaryControl distTube;
    RotaryControl distMech;
    RotaryControl distBit;
    RotaryControl distXcita;
    RotaryControl distMix;

    // 3. MM Filter
    RotaryControl mmCutoff;
    RotaryControl mmRes;
    RotaryControl mmEnvAmount;
    RotaryControl mmEnvAttack;
    RotaryControl mmEnvRelease;
    juce::ComboBox mmModeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mmModeAttachment;

    // 4. Flanger
    RotaryControl flangerRate;
    RotaryControl flangerDepth;
    RotaryControl flangerFeedback;
    RotaryControl flangerMix;

    // 5. Magic EQ
    RotaryControl magicFreq;
    RotaryControl magicAmount;

    // 6. Compressor
    RotaryControl compThresh;
    RotaryControl compRatio;
    RotaryControl compAttack;
    RotaryControl compRelease;
    RotaryControl compMix;
    juce::ToggleButton compPhatToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> compPhatAttachment;

    // 7. Master
    RotaryControl masterMix;
    RotaryControl masterGain;

    // Help Floating Bubble
    camelphat::ui::HelpOverlayComponent helpOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CamelPhatAudioProcessorEditor)
};
