#include "ReverbDecayDisplayComponent.h"
#include <cmath>

namespace wvzz::ui {

ReverbDecayDisplayComponent::ReverbDecayDisplayComponent(const dsp::ReverbDecayState& stateToShow)
    : state(stateToShow)
{
}

float ReverbDecayDisplayComponent::decayDbAt(float timeSec, float predelaySec, float decaySec, float extraDbPerSecond) noexcept
{
    if (timeSec < predelaySec)
        return -100.0f;
    const float t = timeSec - predelaySec;
    return -(60.0f / juce::jmax(0.01f, decaySec) + juce::jmax(0.0f, extraDbPerSecond)) * t;
}

float ReverbDecayDisplayComponent::dampingDbPerSecond(float frequencyHz, float cornerHz, float fullDampingDbPerSecond, bool highPass) noexcept
{
    // Share of the signal at this frequency that lies in the damped range of a one-pole split
    const float ratio = highPass ? (cornerHz / juce::jmax(1.0f, frequencyHz)) : (frequencyHz / juce::jmax(1.0f, cornerHz));
    const float share = (ratio * ratio) / (1.0f + ratio * ratio);
    return juce::jmax(0.0f, fullDampingDbPerSecond) * share;
}

void ReverbDecayDisplayComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(WvZzLookAndFeel::cardBg.darker(0.25f));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    const float decay     = state.decaySeconds.load(std::memory_order_relaxed);
    const float predelay  = state.predelayMs.load(std::memory_order_relaxed) * 0.001f;
    const float hiHz      = state.hiDampHz.load(std::memory_order_relaxed);
    const float loHz      = state.loDampHz.load(std::memory_order_relaxed);
    const float earlyMs   = state.earlyMs.load(std::memory_order_relaxed);
    const float shimmer   = state.shimmer.load(std::memory_order_relaxed);
    const float wetLevel  = state.wetLevel.load(std::memory_order_relaxed);
    const int mode        = state.mode.load(std::memory_order_relaxed);
    const bool active     = state.active.load(std::memory_order_relaxed);

    auto area = bounds.reduced(8.0f, 6.0f);
    const float alpha = active ? 1.0f : 0.35f;

    // Header
    static const char* modeNames[] = { "PLATE", "HALL", "SHIMMER" };
    auto header = area.removeFromTop(16.0f);
    g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    g.setColour(active ? WvZzLookAndFeel::textPrimary : WvZzLookAndFeel::textSecondary);
    juce::String title = modeNames[juce::jlimit(0, 2, mode)];
    if (!active)
        title += "  (BYPASSED: TAIL ONLY)";
    g.drawText(title, header, juce::Justification::centredLeft);
    g.setColour(WvZzLookAndFeel::textSecondary);
    g.drawText("DECAY " + juce::String(decay, 2) + " s   PRE " + juce::String(juce::roundToInt(predelay * 1000.0f)) + " ms",
               header, juce::Justification::centredRight);

    auto footer = area.removeFromBottom(14.0f);
    area.removeFromTop(4.0f);

    const float plotX = area.getX() + 26.0f;
    const float plotW = area.getWidth() - 30.0f;
    const float plotY = area.getY();
    const float plotH = area.getHeight();

    // Time axis: the whole mid decay plus some room
    const float axisSec = juce::jlimit(0.5f, 30.0f, (predelay + decay) * 1.15f);
    auto xFor = [&](float t) { return plotX + plotW * juce::jlimit(0.0f, 1.0f, t / axisSec); };
    auto yFor = [&](float db) { return plotY + plotH * juce::jlimit(0.0f, 1.0f, -db / 60.0f); };

    // Grid: dB lines
    g.setFont(juce::FontOptions(8.5f));
    for (int db = 0; db >= -60; db -= 20)
    {
        const float y = yFor(static_cast<float>(db));
        g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.3f));
        g.drawHorizontalLine(static_cast<int>(y), plotX, plotX + plotW);
        g.setColour(WvZzLookAndFeel::textSecondary.withAlpha(0.7f));
        g.drawText(juce::String(db), juce::Rectangle<float>(area.getX(), y - 6.0f, 22.0f, 12.0f), juce::Justification::centredRight);
    }

    // Grid: time lines
    const float step = (axisSec > 12.0f) ? 5.0f : ((axisSec > 5.0f) ? 2.0f : ((axisSec > 2.0f) ? 1.0f : 0.25f));
    for (float t = 0.0f; t <= axisSec; t += step)
    {
        const float x = xFor(t);
        g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.25f));
        g.drawVerticalLine(static_cast<int>(x), plotY, plotY + plotH);
        g.setColour(WvZzLookAndFeel::textSecondary.withAlpha(0.7f));
        g.drawText(juce::String(t, (step < 1.0f) ? 2 : 0) + " s",
                   juce::Rectangle<float>(x - 24.0f, footer.getY(), 48.0f, footer.getHeight()), juce::Justification::centred);
    }

    // Dry sound at time 0
    g.setColour(WvZzLookAndFeel::textPrimary.withAlpha(0.85f * alpha));
    g.fillRect(juce::Rectangle<float>(plotX - 1.5f, plotY, 3.0f, plotH));

    // Early reflections: a block from 0 to the pattern length
    if (earlyMs > 0.5f)
    {
        const float x1 = xFor(earlyMs * 0.001f);
        g.setColour(WvZzLookAndFeel::emeraldGreen.withAlpha(0.22f * alpha));
        g.fillRect(juce::Rectangle<float>(plotX, yFor(-6.0f), juce::jmax(3.0f, x1 - plotX), plotY + plotH - yFor(-6.0f)));
    }

    // Decay lines: low (100 Hz), mid, high (8 kHz)
    // Same model as the DSP: the damped range decays in 35% of the Decay time; end positions switch damping off
    const float fullDamping = 60.0f / (0.35f * juce::jmax(0.05f, decay)) - 60.0f / juce::jmax(0.05f, decay);
    const float lowExtra  = (loHz > 20.5f)    ? dampingDbPerSecond(100.0f, loHz, fullDamping, true) : 0.0f;
    const float highExtra = (hiHz < 19990.0f) ? dampingDbPerSecond(8000.0f, hiHz, fullDamping, false) : 0.0f;

    auto drawLine = [&](float extra, juce::Colour colour, float thickness) {
        juce::Path path;
        bool started = false;
        const int points = juce::jmax(2, static_cast<int>(plotW / 2.0f));
        for (int i = 0; i <= points; ++i)
        {
            const float t = axisSec * static_cast<float>(i) / static_cast<float>(points);
            const float db = decayDbAt(t, predelay, decay, extra);
            if (db < -99.0f)
                continue;
            const float x = xFor(t);
            const float y = yFor(juce::jmax(-60.0f, db));
            if (!started) { path.startNewSubPath(xFor(predelay), yFor(0.0f)); started = true; }
            path.lineTo(x, y);
            if (db <= -60.0f)
                break;
        }
        g.setColour(colour.withAlpha(alpha));
        g.strokePath(path, juce::PathStrokeType(thickness));
    };

    drawLine(lowExtra,  WvZzLookAndFeel::warmAmber, 1.4f);
    drawLine(highExtra, WvZzLookAndFeel::electricViolet, 1.4f);
    drawLine(0.0f,      WvZzLookAndFeel::skyCyan, 2.2f);

    // Legend
    g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    auto legend = juce::Rectangle<float>(plotX + plotW - 150.0f, plotY + 2.0f, 148.0f, 12.0f);
    g.setColour(WvZzLookAndFeel::warmAmber.withAlpha(alpha));
    g.drawText("LOW", legend.removeFromLeft(36.0f), juce::Justification::centred);
    g.setColour(WvZzLookAndFeel::skyCyan.withAlpha(alpha));
    g.drawText("MID", legend.removeFromLeft(36.0f), juce::Justification::centred);
    g.setColour(WvZzLookAndFeel::electricViolet.withAlpha(alpha));
    g.drawText("HIGH", legend.removeFromLeft(36.0f), juce::Justification::centred);
    if (mode == 2 && shimmer > 0.01f)
    {
        g.setColour(WvZzLookAndFeel::electricViolet.withAlpha(alpha));
        g.drawText("+SHIM", legend, juce::Justification::centred);
    }

    // Wet level meter along the bottom edge
    g.setColour(WvZzLookAndFeel::emeraldGreen.withAlpha(0.6f * alpha));
    g.fillRect(juce::Rectangle<float>(plotX, plotY + plotH - 2.0f, plotW * juce::jlimit(0.0f, 1.0f, wetLevel), 2.0f));
}

} // namespace wvzz::ui
