#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "CamelPhatLookAndFeel.h"

namespace camelphat::ui {

/**
 * @brief Standardized container card for DSP modules.
 * Renders consistent dark/neon card chrome with header title, accent badge, and content bounds.
 */
class ModuleCardComponent : public juce::Component
{
public:
    ModuleCardComponent(const juce::String& sectionTitle, juce::Colour accent = CamelPhatLookAndFeel::neonGreen)
        : title(sectionTitle), accentColor(accent)
    {
    }

    ~ModuleCardComponent() override = default;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        // 1. Card Background
        g.setColour(CamelPhatLookAndFeel::cardBg);
        g.fillRoundedRectangle(bounds, 6.0f);

        // 2. Card Border
        g.setColour(CamelPhatLookAndFeel::cardBorder);
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

        // 3. Header Bar Area
        auto headerArea = bounds.removeFromTop(28.0f);
        headerArea.reduce(8.0f, 0.0f);

        // Accent indicator dot
        const float dotSize = 6.0f;
        const float dotY = headerArea.getCentreY() - dotSize * 0.5f;
        g.setColour(accentColor);
        g.fillEllipse(headerArea.getX(), dotY, dotSize, dotSize);

        // Title text
        g.setColour(CamelPhatLookAndFeel::textPrimary);
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        auto titleArea = headerArea.withTrimmedLeft(12.0f);
        g.drawText(title, titleArea, juce::Justification::centredLeft, true);

        // Divider line below header
        g.setColour(CamelPhatLookAndFeel::cardBorder.withAlpha(0.6f));
        g.drawHorizontalLine(28, bounds.getX(), bounds.getRight());
    }

    [[nodiscard]] juce::Rectangle<int> getContentBounds() const noexcept
    {
        return getBounds().reduced(8).withTrimmedTop(32);
    }

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModuleCardComponent)
};

} // namespace camelphat::ui
