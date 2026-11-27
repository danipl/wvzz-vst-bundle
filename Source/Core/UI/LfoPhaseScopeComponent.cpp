#include "LfoPhaseScopeComponent.h"
#include <cmath>

namespace wvzz::ui {

LfoPhaseScopeComponent::LfoPhaseScopeComponent(std::vector<Lane> lanesToShow)
    : lanes(std::move(lanesToShow))
{
    setOpaque(false);
}

float LfoPhaseScopeComponent::waveformValue(int shape, float position01) noexcept
{
    const float n = position01 - std::floor(position01);
    switch (shape)
    {
        case dsp::LfoPhaseTap::Triangle: return (n < 0.5f) ? (4.0f * n - 1.0f) : (3.0f - 4.0f * n);
        case dsp::LfoPhaseTap::Square:   return (n < 0.5f) ? 1.0f : -1.0f;
        default:                         return std::sin(n * juce::MathConstants<float>::twoPi);
    }
}

void LfoPhaseScopeComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(WvZzLookAndFeel::cardBg.darker(0.25f));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    if (lanes.empty())
        return;

    auto area = bounds.reduced(6.0f, 4.0f);
    const float laneHeight = area.getHeight() / static_cast<float>(lanes.size());
    constexpr float labelWidth = 62.0f;

    for (size_t laneIndex = 0; laneIndex < lanes.size(); ++laneIndex)
    {
        const auto& lane = lanes[laneIndex];
        auto laneArea = area.removeFromTop(laneHeight).reduced(0.0f, 3.0f);

        const bool active = (lane.tap != nullptr) && lane.tap->active.load(std::memory_order_relaxed);
        const float phase  = (lane.tap != nullptr) ? lane.tap->phase.load(std::memory_order_relaxed) : 0.0f;
        const float offset = (lane.tap != nullptr) ? lane.tap->stereoOffset.load(std::memory_order_relaxed) : 0.0f;
        const float depth  = (lane.tap != nullptr) ? lane.tap->depth.load(std::memory_order_relaxed) : 0.0f;
        const int shape    = (lane.tap != nullptr) ? lane.tap->shape.load(std::memory_order_relaxed) : 0;

        const float alpha = active ? 1.0f : 0.28f;
        const auto colour = lane.colour.withMultipliedAlpha(alpha);

        // Lane label and state
        auto labelArea = laneArea.removeFromLeft(labelWidth);
        g.setColour(active ? WvZzLookAndFeel::textPrimary : WvZzLookAndFeel::textSecondary.withAlpha(0.6f));
        g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
        g.drawText(lane.name, labelArea.removeFromTop(labelArea.getHeight() * 0.5f), juce::Justification::bottomLeft);
        g.setColour(WvZzLookAndFeel::textSecondary.withAlpha(active ? 0.9f : 0.5f));
        g.setFont(juce::FontOptions(9.0f));
        g.drawText(active ? "ACTIVE" : "BYPASSED", labelArea, juce::Justification::topLeft);

        // Plot frame and center line
        auto plot = laneArea.reduced(2.0f, 0.0f);
        g.setColour(WvZzLookAndFeel::cardBg.darker(0.45f));
        g.fillRoundedRectangle(plot, 3.0f);
        const float midY = plot.getCentreY();
        g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.45f));
        g.drawHorizontalLine(static_cast<int>(midY), plot.getX() + 2.0f, plot.getRight() - 2.0f);

        // Waveform over one cycle. Depth scales the height; a small minimum keeps the shape visible.
        const float amplitude = plot.getHeight() * 0.40f * (0.25f + 0.75f * juce::jlimit(0.0f, 1.0f, depth));
        const float plotX = plot.getX() + 4.0f;
        const float plotW = plot.getWidth() - 8.0f;

        juce::Path wave;
        const int numPoints = juce::jmax(2, static_cast<int>(plotW));
        for (int i = 0; i <= numPoints; ++i)
        {
            const float pos = static_cast<float>(i) / static_cast<float>(numPoints);
            const float x = plotX + pos * plotW;
            const float y = midY - waveformValue(shape, pos) * amplitude;
            if (i == 0) wave.startNewSubPath(x, y);
            else        wave.lineTo(x, y);
        }
        g.setColour(colour.withMultipliedAlpha(0.7f));
        g.strokePath(wave, juce::PathStrokeType(1.4f));

        // Markers: left channel filled, right channel ring
        const float leftPos  = phase - std::floor(phase);
        const float rightRaw = phase + offset;
        const float rightPos = rightRaw - std::floor(rightRaw);

        const float lx = plotX + leftPos * plotW;
        const float ly = midY - waveformValue(shape, leftPos) * amplitude;
        const float rx = plotX + rightPos * plotW;
        const float ry = midY - waveformValue(shape, rightPos) * amplitude;

        g.setColour(colour.withMultipliedAlpha(0.35f));
        g.drawVerticalLine(static_cast<int>(lx), plot.getY() + 2.0f, plot.getBottom() - 2.0f);

        g.setColour(colour);
        g.drawEllipse(rx - 4.0f, ry - 4.0f, 8.0f, 8.0f, 1.5f);
        g.fillEllipse(lx - 3.5f, ly - 3.5f, 7.0f, 7.0f);
    }
}

} // namespace wvzz::ui
