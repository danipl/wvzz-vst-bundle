#pragma once

#include "../../Core/Interfaces/IModuleView.h"
#include "../../Core/UI/ModuleCardComponent.h"
#include "../../Core/UI/HelpButtonComponent.h"
#include "../../Core/UI/WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace wvzz::modules {

class MMFilterView : public ui::IModuleView
{
public:
    MMFilterView(juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& prefix,
                 ui::IHelpOverlayHost* helpHost);
    ~MMFilterView() override = default;

    void resized() override;
    void paint(juce::Graphics& g) override;

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

    void initRotary(RotaryControl& rc, const juce::String& paramId, const juce::String& labelText,
                    bool isBipolar = false, juce::Colour accent = ui::WvZzLookAndFeel::neonGreen);

    juce::AudioProcessorValueTreeState& apvtsState;
    juce::String instancePrefix;

    ui::ModuleCardComponent card;

    ToggleControl bypassToggle;
    std::unique_ptr<ui::HelpButtonComponent> helpButton;

    ComboControl typeBox;

    RotaryControl cutoffKnob;
    RotaryControl resKnob;
    RotaryControl mixKnob;

    RotaryControl envAttKnob;
    RotaryControl envRelKnob;
    RotaryControl envDepthKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MMFilterView)
};

} // namespace wvzz::modules
