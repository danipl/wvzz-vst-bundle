#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "WvZzLookAndFeel.h"

namespace wvzz::ui {

/**
 * @brief Standardized container card for DSP modules.
 * Renders consistent dark/neon card chrome with header title, accent badge,
 * bypass state dimming, and content bounds.
 */
class ModuleCardComponent : public juce::Component
{
public:
    ModuleCardComponent(const juce::String& sectionTitle, juce::Colour accent = WvZzLookAndFeel::neonGreen)
        : title(sectionTitle), accentColor(accent)
    {
    }

    ~ModuleCardComponent() override = default;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(0.5f);

        // 1. Translucent Dark Glass Card Background
        const auto topBg = isBypassed ? WvZzLookAndFeel::cardBg.darker(0.25f) : WvZzLookAndFeel::cardBg;
        const auto botBg = isBypassed ? WvZzLookAndFeel::cardBg.darker(0.45f) : WvZzLookAndFeel::cardBg.darker(0.2f);
        juce::ColourGradient bgGrad(topBg, bounds.getCentreX(), bounds.getY(),
                                    botBg, bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds, 6.0f);

        // 2. Refined Hairline Border with Top-Down Light Catch
        const auto topBorder = isBypassed ? WvZzLookAndFeel::cardHighlight.withAlpha(0.2f)
                                          : WvZzLookAndFeel::cardHighlight.withAlpha(0.55f);
        const auto botBorder = isBypassed ? WvZzLookAndFeel::cardBorder.withAlpha(0.2f)
                                          : WvZzLookAndFeel::cardBorder.withAlpha(0.35f);
        juce::ColourGradient borderGrad(topBorder, bounds.getCentreX(), bounds.getY(),
                                        botBorder, bounds.getCentreX(), bounds.getBottom(), false);
        g.setGradientFill(borderGrad);
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

        // 3. Header Bar Area
        auto headerArea = bounds.removeFromTop(28.0f);
        headerArea.reduce(10.0f, 0.0f);

        // Luminous accent indicator dot with soft aura (dimmed when bypassed)
        const float dotSize = 5.0f;
        const float dotY = headerArea.getCentreY() - dotSize * 0.5f;
        const float dotX = headerArea.getX();

        if (!isBypassed)
        {
            g.setColour(accentColor.withAlpha(0.25f));
            g.fillEllipse(dotX - 1.5f, dotY - 1.5f, dotSize + 3.0f, dotSize + 3.0f);
        }

        g.setColour(isBypassed ? WvZzLookAndFeel::textSecondary.withAlpha(0.4f) : accentColor);
        g.fillEllipse(dotX, dotY, dotSize, dotSize);

        // Clean, letter-spaced uppercase title text
        g.setColour(isBypassed ? WvZzLookAndFeel::textSecondary.withAlpha(0.5f) : WvZzLookAndFeel::textPrimary);
        g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
        auto titleArea = headerArea.withTrimmedLeft(12.0f);
        g.drawText(title, titleArea, juce::Justification::centredLeft, true);

        // Note: Hard horizontal divider line intentionally removed for sleek modern dark glass aesthetic
    }

    [[nodiscard]] juce::Rectangle<int> getContentBounds() const noexcept
    {
        return getLocalBounds().reduced(8).withTrimmedTop(30);
    }

    /**
     * @brief Returns the bounds for the header bypass button in local coordinates.
     */
    [[nodiscard]] juce::Rectangle<int> getHeaderBypassBounds() const noexcept
    {
        return juce::Rectangle<int>(getWidth() - 72, 5, 42, 18);
    }

    /**
     * @brief Returns the bounds for the header ? help button in local coordinates.
     */
    [[nodiscard]] juce::Rectangle<int> getHeaderHelpBounds() const noexcept
    {
        return juce::Rectangle<int>(getWidth() - 25, 6, 16, 16);
    }

    void setBypassed(bool bypassed)
    {
        if (isBypassed != bypassed)
        {
            isBypassed = bypassed;
            repaint();
        }
    }

    [[nodiscard]] bool getBypassed() const noexcept { return isBypassed; }

    void setTitle(const juce::String& newTitle)
    {
        title = newTitle;
        repaint();
    }

    void setAccentColor(juce::Colour newColor)
    {
        accentColor = newColor;
        repaint();
    }

private:
    juce::String title;
    juce::Colour accentColor;
    bool isBypassed { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleCardComponent)
};

} // namespace wvzz::ui
