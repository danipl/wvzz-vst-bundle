#include "DelayTapDisplayComponent.h"
#include <cmath>

namespace wvzz::ui {

DelayTapDisplayComponent::DelayTapDisplayComponent(const dsp::DelayTapState& stateToShow)
    : state(stateToShow)
{
}

std::vector<DelayTapDisplayComponent::Tap> DelayTapDisplayComponent::computeTaps(float timeLeftMs, float timeRightMs,
                                                                                 float feedback, bool pingPong)
{
    std::vector<Tap> taps;
    taps.reserve(static_cast<size_t>(maxTaps) * 2);
    const float fb = juce::jlimit(0.0f, 1.0f, feedback); // above 100% the soft clipper holds the level

    if (pingPong)
    {
        // The signal crosses sides on every repeat: left line (left time), then right line (right time), ...
        float level = 1.0f;
        float time = 0.0f;
        for (int n = 1; n <= maxTaps && level > 0.01f; ++n)
        {
            const bool right = (n % 2) == 0;
            time += right ? timeRightMs : timeLeftMs;
            taps.push_back({ time, level, right });
            level *= fb;
        }
        return taps;
    }

    for (int channel = 0; channel < 2; ++channel)
    {
        const float time = (channel == 0) ? timeLeftMs : timeRightMs;
        float level = 1.0f;
        for (int n = 1; n <= maxTaps && level > 0.01f; ++n)
        {
            taps.push_back({ time * static_cast<float>(n), level, channel == 1 });
            level *= fb;
        }
    }
    return taps;
}

void DelayTapDisplayComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(WvZzLookAndFeel::cardBg.darker(0.25f));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    const float timeL    = state.timeLeftMs.load(std::memory_order_relaxed);
    const float timeR    = state.timeRightMs.load(std::memory_order_relaxed);
    const float feedback = state.feedback.load(std::memory_order_relaxed);
    const float duckGain = state.duckGain.load(std::memory_order_relaxed);
    const float wetLevel = state.wetLevel.load(std::memory_order_relaxed);
    const int mode       = state.mode.load(std::memory_order_relaxed);
    const bool pingPong  = state.pingPong.load(std::memory_order_relaxed);
    const bool active    = state.active.load(std::memory_order_relaxed);

    auto area = bounds.reduced(8.0f, 6.0f);

    // Header line: mode, time, feedback
    static const char* modeNames[] = { "CLEAN", "TAPE", "BBD", "GRANULAR" };
    auto header = area.removeFromTop(16.0f);
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.setColour(active ? WvZzLookAndFeel::textPrimary : WvZzLookAndFeel::textSecondary);
    juce::String title = juce::String(modeNames[juce::jlimit(0, 3, mode)]) + (pingPong ? "  PING-PONG" : "  STEREO");
    if (!active)
        title += "  (BYPASSED: TAIL ONLY)";
    g.drawText(title, header, juce::Justification::centredLeft);
    g.setColour(WvZzLookAndFeel::textSecondary);
    g.drawText(juce::String(timeL, 0) + " ms   FB " + juce::String(juce::roundToInt(feedback * 100.0f)) + "%",
               header, juce::Justification::centredRight);

    // Footer: time axis labels
    auto footer = area.removeFromBottom(14.0f);
    area.removeFromTop(4.0f);

    // Time axis length: enough for the visible repeats, at least 1 second
    const auto taps = computeTaps(timeL, timeR, feedback, pingPong);
    float lastTime = 1000.0f;
    for (const auto& tap : taps)
        lastTime = juce::jmax(lastTime, tap.timeMs);
    const float axisMs = juce::jmin(16000.0f, lastTime * 1.08f);

    const float midY = area.getCentreY();
    const float laneH = area.getHeight() * 0.5f - 4.0f;
    const float alpha = active ? 1.0f : 0.35f;

    // Lane labels and centre line
    g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    g.setColour(WvZzLookAndFeel::textSecondary.withAlpha(0.8f));
    g.drawText("L", area.withHeight(12.0f), juce::Justification::topLeft);
    g.drawText("R", area.withTop(area.getBottom() - 12.0f), juce::Justification::bottomLeft);
    g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.6f));
    g.drawHorizontalLine(static_cast<int>(midY), area.getX(), area.getRight());

    const float plotX = area.getX() + 14.0f;
    const float plotW = area.getWidth() - 18.0f;

    // Grid: one line per second (or per 250 ms on short axes)
    const float gridStep = (axisMs > 4000.0f) ? 1000.0f : ((axisMs > 1500.0f) ? 500.0f : 250.0f);
    g.setFont(juce::FontOptions(8.5f));
    for (float t = 0.0f; t <= axisMs; t += gridStep)
    {
        const float x = plotX + plotW * (t / axisMs);
        g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.25f));
        g.drawVerticalLine(static_cast<int>(x), area.getY(), area.getBottom());
        g.setColour(WvZzLookAndFeel::textSecondary.withAlpha(0.7f));
        g.drawText((t >= 1000.0f) ? juce::String(t / 1000.0f, 1) + " s" : juce::String(juce::roundToInt(t)) + " ms",
                   juce::Rectangle<float>(x - 24.0f, footer.getY(), 48.0f, footer.getHeight()), juce::Justification::centred);
    }

    // Dry signal marker at time 0
    g.setColour(WvZzLookAndFeel::textPrimary.withAlpha(0.8f * alpha));
    g.fillRect(juce::Rectangle<float>(plotX - 1.5f, midY - laneH, 3.0f, laneH * 2.0f));

    // Repeats: left above the centre line, right below
    for (const auto& tap : taps)
    {
        const float x = plotX + plotW * (tap.timeMs / axisMs);
        const float h = juce::jmax(2.0f, laneH * tap.level * duckGain);
        const auto colour = (tap.rightChannel ? WvZzLookAndFeel::warmAmber : WvZzLookAndFeel::skyCyan).withAlpha(alpha);

        g.setColour(colour.withMultipliedAlpha(0.85f));
        if (tap.rightChannel)
            g.fillRoundedRectangle(x - 2.0f, midY + 1.0f, 4.0f, h, 1.5f);
        else
            g.fillRoundedRectangle(x - 2.0f, midY - 1.0f - h, 4.0f, h, 1.5f);
    }

    // Wet level meter along the bottom edge of the plot
    const float meterW = plotW * juce::jlimit(0.0f, 1.0f, wetLevel);
    g.setColour(WvZzLookAndFeel::emeraldGreen.withAlpha(0.6f * alpha));
    g.fillRect(juce::Rectangle<float>(plotX, area.getBottom() - 2.0f, meterW, 2.0f));
}

} // namespace wvzz::ui
