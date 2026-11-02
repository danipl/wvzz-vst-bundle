#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "WvZzLookAndFeel.h"

namespace wvzz::ui {

/**
 * @brief Scalable vector brand logo component.
 * Renders the geometric WvZz waveform emblem and subtitle badge with resolution-independent
 * vector paths (juce::Path) scaling sharply across 75% to 200% window dimensions.
 */
class WvZzLogoComponent : public juce::Component
{
public:
    explicit WvZzLogoComponent(const juce::String& subtitleText = "DISTORTION")
        : subtitle(subtitleText)
    {
        setInterceptsMouseClicks(false, false);
    }

    ~WvZzLogoComponent() override = default;

    void setSubtitle(const juce::String& newSubtitle)
    {
        subtitle = newSubtitle;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty())
            return;

        // Scaling factor relative to nominal height of 36px
        const float targetHeight = bounds.getHeight();
        const float scale = targetHeight / 36.0f;

        // Emblem area on the left
        const float emblemSize = 28.0f * scale;
        const float emblemX = bounds.getX() + 4.0f * scale;
        const float emblemY = bounds.getCentreY() - emblemSize * 0.5f;

        // 1. Draw glowing background badge for emblem
        auto badgeRect = juce::Rectangle<float>(emblemX, emblemY, emblemSize, emblemSize);
        g.setColour(WvZzLookAndFeel::cardBg.darker(0.3f));
        g.fillRoundedRectangle(badgeRect, 5.0f * scale);
        g.setColour(WvZzLookAndFeel::cardBorder);
        g.drawRoundedRectangle(badgeRect, 5.0f * scale, 1.0f * scale);

        // 2. Draw geometric WvZz waveform / lightning monogram vector path
        juce::Path emblem;
        const float bx = emblemX + 4.0f * scale;
        const float by = emblemY + 5.0f * scale;
        const float bw = emblemSize - 8.0f * scale;
        const float bh = emblemSize - 10.0f * scale;

        emblem.startNewSubPath(bx, by + bh * 0.2f);
        emblem.lineTo(bx + bw * 0.22f, by + bh * 0.85f);  // First W drop
        emblem.lineTo(bx + bw * 0.42f, by + bh * 0.35f);  // W mid peak
        emblem.lineTo(bx + bw * 0.58f, by + bh * 0.85f);  // Second W drop
        emblem.lineTo(bx + bw * 0.72f, by + bh * 0.15f);  // Rise to Z top
        emblem.lineTo(bx + bw,         by + bh * 0.15f);  // Z top bar
        emblem.lineTo(bx + bw * 0.65f, by + bh * 0.85f);  // Z diagonal
        emblem.lineTo(bx + bw,         by + bh * 0.85f);  // Z bottom bar

        juce::ColourGradient grad(WvZzLookAndFeel::neonCyan, bx, by,
                                  WvZzLookAndFeel::neonGreen, bx + bw, by + bh, false);
        g.setGradientFill(grad);
        g.strokePath(emblem, juce::PathStrokeType(2.2f * scale, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 3. Draw Brand Typography: "WvZz"
        const float textX = emblemX + emblemSize + 10.0f * scale;
        const float brandFontSize = 20.0f * scale;
        juce::Font brandFont(juce::FontOptions(brandFontSize, juce::Font::bold));
        g.setFont(brandFont);
        g.setColour(WvZzLookAndFeel::textPrimary);

        const float brandW = brandFont.getStringWidth("WvZz");
        auto brandArea = juce::Rectangle<float>(textX, bounds.getY(), brandW, bounds.getHeight());
        g.drawText("WvZz", brandArea, juce::Justification::centredLeft, false);

        // 4. Draw Subtitle Badge (e.g. "DISTORTION")
        if (subtitle.isNotEmpty())
        {
            const float subFontSize = 9.0f * scale;
            const float subPadX = 6.0f * scale;
            const float subPadY = 3.0f * scale;

            juce::Font subFont(juce::FontOptions(subFontSize, juce::Font::bold));
            const float textW = subFont.getStringWidth(subtitle) + subPadX * 2.0f;
            const float textH = subFontSize + subPadY * 2.0f;

            const float subX = textX + brandW + 10.0f * scale;
            const float subY = bounds.getCentreY() - textH * 0.5f;

            auto subRect = juce::Rectangle<float>(subX, subY, textW, textH);
            g.setColour(WvZzLookAndFeel::neonAmber.withAlpha(0.18f));
            g.fillRoundedRectangle(subRect, 3.0f * scale);
            g.setColour(WvZzLookAndFeel::neonAmber.withAlpha(0.7f));
            g.drawRoundedRectangle(subRect, 3.0f * scale, 1.0f * scale);

            g.setFont(subFont);
            g.setColour(WvZzLookAndFeel::neonAmber);
            g.drawText(subtitle, subRect, juce::Justification::centred, false);
        }
    }

private:
    juce::String subtitle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WvZzLogoComponent)
};

} // namespace wvzz::ui
