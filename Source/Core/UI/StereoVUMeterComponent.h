#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Interfaces/IDSPModule.h"

namespace wvzz::ui {

/**
 * @brief High-precision 60 FPS stereo peak and RMS level meter.
 * 
 * Satisfies the Interface Segregation Principle by depending strictly
 * on the wvzz::IMeterSource read-only contract.
 */
class StereoVUMeterComponent : public juce::Component,
                               private juce::Timer
{
public:
    explicit StereoVUMeterComponent(const IMeterSource& meterSource, juce::String meterLabel = {})
        : source(meterSource), label(std::move(meterLabel))
    {
        startTimerHz(60);
    }

    ~StereoVUMeterComponent() override
    {
        stopTimer();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background track
        g.setColour(juce::Colour(0xff0d1117));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xff21262d));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        if (label.isNotEmpty())
        {
            auto labelArea = bounds.removeFromBottom(12.0f);
            g.setColour(juce::Colour(0xff8b949e));
            g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
            g.drawText(label, labelArea, juce::Justification::centred, false);
        }

        auto inner = bounds.reduced(3.0f);
        float channelWidth = (inner.getWidth() - 2.0f) * 0.5f;

        drawChannel(g, inner.removeFromLeft(channelWidth), currentPeakLeft, currentRmsLeft, peakHoldLeft);
        inner.removeFromLeft(2.0f); // Gap between L and R
        drawChannel(g, inner, currentPeakRight, currentRmsRight, peakHoldRight);
    }

private:
    void timerCallback() override
    {
        // Atomically query current levels from the DSP meter source
        float rawPeakL = source.getPeakLevel(0);
        float rawPeakR = source.getPeakLevel(1);
        float rawRmsL  = source.getRmsLevel(0);
        float rawRmsR  = source.getRmsLevel(1);

        // Ballistics: fast attack, smooth decay
        constexpr float decay = 0.88f;
        currentPeakLeft  = juce::jmax(rawPeakL, currentPeakLeft * decay);
        currentPeakRight = juce::jmax(rawPeakR, currentPeakRight * decay);
        currentRmsLeft   = juce::jmax(rawRmsL, currentRmsLeft * decay);
        currentRmsRight  = juce::jmax(rawRmsR, currentRmsRight * decay);

        // Peak hold decay
        constexpr float peakHoldDecay = 0.96f;
        if (currentPeakLeft > peakHoldLeft)
            peakHoldLeft = currentPeakLeft;
        else
            peakHoldLeft *= peakHoldDecay;

        if (currentPeakRight > peakHoldRight)
            peakHoldRight = currentPeakRight;
        else
            peakHoldRight *= peakHoldDecay;

        repaint();
    }

    void drawChannel(juce::Graphics& g, juce::Rectangle<float> area,
                     float peak, float rms, float peakHold)
    {
        // Dark track channel
        g.setColour(juce::Colour(0xff161b22));
        g.fillRect(area);

        // Convert linear amplitude to normalized height [0..1]
        // Range: -60 dB to 0 dB
        auto levelToY = [](float linear) -> float {
            if (linear <= 0.0001f) return 0.0f;
            float dB = juce::Decibels::gainToDecibels(linear, -60.0f);
            return juce::jlimit(0.0f, 1.0f, (dB + 60.0f) / 60.0f);
        };

        float peakHeight = levelToY(peak) * area.getHeight();
        float rmsHeight  = levelToY(rms) * area.getHeight();
        float holdY      = area.getBottom() - levelToY(peakHold) * area.getHeight();

        // 1. RMS bar (solid neon green / cyan)
        auto rmsRect = area.removeFromBottom(rmsHeight);
        g.setColour(juce::Colour(0xff00e676).withAlpha(0.6f));
        g.fillRect(rmsRect);

        // 2. Peak bar (brighter, semi-transparent top cap)
        auto peakRect = area.removeFromBottom(peakHeight - rmsHeight);
        g.setColour(juce::Colour(0xff00e5ff));
        g.fillRect(peakRect);

        // 3. Peak hold needle
        if (holdY >= area.getY() && holdY <= area.getBottom())
        {
            g.setColour(juce::Colour(0xffff9100)); // Neon Amber peak tick
            g.drawHorizontalLine(static_cast<int>(holdY), area.getX(), area.getRight());
        }
    }

    const IMeterSource& source;
    juce::String label;

    float currentPeakLeft  { 0.0f };
    float currentPeakRight { 0.0f };
    float currentRmsLeft   { 0.0f };
    float currentRmsRight  { 0.0f };
    float peakHoldLeft     { 0.0f };
    float peakHoldRight    { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoVUMeterComponent)
};

} // namespace wvzz::ui
