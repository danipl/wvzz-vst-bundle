#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <memory>

#include "PluginProcessor.h"
#include "Core/UI/WvZzLookAndFeel.h"
#include "Core/UI/WvZzLogoComponent.h"
#include "Core/UI/IHelpSystem.h"
#include "Core/UI/HelpContentRepository.h"
#include "Core/UI/HelpOverlayComponent.h"
#include "Core/Interfaces/IModuleView.h"

namespace wvzz {

class WvZzReverbAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        public ui::IHelpOverlayHost,
                                        private juce::Timer
{
public:
    explicit WvZzReverbAudioProcessorEditor(WvZzReverbAudioProcessor& proc);
    ~WvZzReverbAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    // IHelpOverlayHost implementation
    void showHelp(const juce::String& topicId, juce::Rectangle<int> targetScreenBounds) override;
    void hideHelp() override;
    [[nodiscard]] bool isHelpVisible() const noexcept override;

private:
    void timerCallback() override;

    class ContentContainer : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
    };

    struct ToggleControl
    {
        juce::ToggleButton button;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    };

    void initToggle(ToggleControl& ctrl, const juce::String& paramId, const juce::String& text,
                    bool isHeaderBypass = false, bool isPill = false,
                    juce::Colour accent = ui::WvZzLookAndFeel::skyCyan);

    WvZzReverbAudioProcessor& audioProcessor;
    ui::WvZzLookAndFeel lookAndFeel;
    ui::HelpContentRepository helpRepo;

    ContentContainer contentContainer;

    // Top Header Bar Controls
    ui::WvZzLogoComponent logoComponent;
    juce::Label subTitleLabel;
    juce::Label signalFlowLabel;
    juce::TextButton prevPresetButton;
    juce::ComboBox presetSelector;
    juce::TextButton nextPresetButton;
    juce::TextButton diceButton;
    ToggleControl masterBypassToggle;

    // Module Views (created polymorphically via IPluginModule::createView)
    std::unique_ptr<ui::IModuleView> reverbView;
    std::unique_ptr<ui::IModuleView> earlyView;
    std::unique_ptr<ui::IModuleView> modView;
    std::unique_ptr<ui::IModuleView> masterView;

    // Help Floating Bubble
    ui::HelpOverlayComponent helpOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WvZzReverbAudioProcessorEditor)
};

} // namespace wvzz
