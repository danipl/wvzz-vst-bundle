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
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        // 1. Card Background
        g.setColour(isBypassed ? WvZzLookAndFeel::cardBg.darker(0.1f) : WvZzLookAndFeel::cardBg);
        g.fillRoundedRectangle(bounds, 6.0f);

        // 2. Card Border
        g.setColour(isBypassed ? WvZzLookAndFeel::cardBorder.withAlpha(0.5f) : WvZzLookAndFeel::cardBorder);
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

        // 3. Header Bar Area
        auto headerArea = bounds.removeFromTop(28.0f);
        headerArea.reduce(8.0f, 0.0f);

        // Accent indicator dot (dimmed when bypassed)
        const float dotSize = 6.0f;
        const float dotY = headerArea.getCentreY() - dotSize * 0.5f;
        g.setColour(isBypassed ? WvZzLookAndFeel::textSecondary.withAlpha(0.5f) : accentColor);
        g.fillEllipse(headerArea.getX(), dotY, dotSize, dotSize);

        // Title text
        g.setColour(isBypassed ? WvZzLookAndFeel::textSecondary : WvZzLookAndFeel::textPrimary);
        g.setFont(juce::FontOptions(12.5f, juce::Font::bold));
        auto titleArea = headerArea.withTrimmedLeft(12.0f);
        g.drawText(title, titleArea, juce::Justification::centredLeft, true);

        // Divider line below header
        g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.6f));
        g.drawHorizontalLine(28, bounds.getX(), bounds.getRight());
    }

    [[nodiscard]] juce::Rectangle<int> getContentBounds() const noexcept
    {
        return getLocalBounds().reduced(8).withTrimmedTop(32);
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
