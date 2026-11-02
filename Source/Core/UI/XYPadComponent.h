#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "WvZzLookAndFeel.h"

namespace wvzz::ui {

/**
 * @brief Interactive 2D vector control pad with crosshairs and draggable puck.
 * Binds to two APVTS parameters simultaneously (e.g. Cutoff vs Resonance, or Tube vs Mech).
 */
class XYPadComponent : public juce::Component,
                       private juce::AudioProcessorValueTreeState::Listener
{
public:
    XYPadComponent(juce::AudioProcessorValueTreeState& state,
                   const juce::String& paramXId,
                   const juce::String& paramYId,
                   const juce::String& title,
                   const juce::String& xLabel,
                   const juce::String& yLabel,
                   juce::Colour padAccent = WvZzLookAndFeel::neonGreen);

    ~XYPadComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setAxisLabels(const juce::String& newXLabel, const juce::String& newYLabel)
    {
        labelX = newXLabel;
        labelY = newYLabel;
        repaint();
    }

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void updateFromMouse(const juce::MouseEvent& e);
    [[nodiscard]] juce::Point<float> getPuckScreenPosition() const noexcept;

    juce::AudioProcessorValueTreeState& apvts;
    juce::String xId;
    juce::String yId;
    juce::RangedAudioParameter* paramX { nullptr };
    juce::RangedAudioParameter* paramY { nullptr };

    juce::String padTitle;
    juce::String labelX;
    juce::String labelY;
    juce::Colour accentColor;

    std::atomic<float> currentNormX { 0.5f };
    std::atomic<float> currentNormY { 0.5f };
    bool isDragging { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XYPadComponent)
};

} // namespace wvzz::ui
