#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "IHelpSystem.h"
#include "CamelPhatLookAndFeel.h"

namespace camelphat::ui {

/**
 * @brief Floating contextual explanation card with auto-positioning and dismissal.
 */
class HelpOverlayComponent : public juce::Component
{
public:
    HelpOverlayComponent()
    {
        setAlwaysOnTop(true);
        setVisible(false);

        closeButton.onClick = [this] { dismiss(); };
        closeButton.setButtonText(juce::CharPointer_UTF8("\xc3\x97")); // Multiplication sign / X
        closeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        closeButton.setColour(juce::TextButton::textColourOffId, CamelPhatLookAndFeel::textSecondary);
        addAndMakeVisible(closeButton);
    }

    ~HelpOverlayComponent() override = default;

    static juce::TextLayout createOverviewLayout(const juce::String& text, float maxWidth)
    {
        juce::AttributedString attr;
        attr.append(text, juce::FontOptions(12.5f, juce::Font::plain), CamelPhatLookAndFeel::textPrimary);
        attr.setWordWrap(juce::AttributedString::WordWrap::byWord);
        attr.setJustification(juce::Justification::topLeft);

        juce::TextLayout layout;
        layout.createLayout(attr, maxWidth);
        return layout;
    }

    void displayTopic(const HelpTopic& topic, juce::Rectangle<int> targetScreenBounds, juce::Component* parentContainer)
    {
        currentTopic = topic;
        setVisible(true);

        const int cardWidth = 360;
        const float contentWidth = static_cast<float>(cardWidth - 28);

        auto overviewLayout = createOverviewLayout(topic.overview, contentWidth);
        const float overviewH = std::ceil(overviewLayout.getHeight());

        int baseHeight = 32 + 16;
        if (topic.signalRole.isNotEmpty())
            baseHeight += 24;

        constexpr float gapBetweenOverviewAndList = 10.0f;
        const int tipsCount = static_cast<int>(topic.parameterTips.size());
        constexpr int tipRowHeight = 24;

        const int cardHeight = baseHeight
                             + static_cast<int>(overviewH)
                             + static_cast<int>(gapBetweenOverviewAndList)
                             + (tipsCount * tipRowHeight)
                             + 16;

        auto parentBounds = parentContainer->getLocalBounds();
        // Convert screen target bounds into parent container's local coordinates
        // Automatically inverts AffineTransform scale on contentContainer
        auto localTargetRect = parentContainer->getLocalArea(nullptr, targetScreenBounds);

        int x = localTargetRect.getX() - 10;
        int y = localTargetRect.getBottom() + 6;

        // Clamp to parent view bounds
        if (x + cardWidth > parentBounds.getRight() - 12)
            x = parentBounds.getRight() - cardWidth - 12;
        if (x < 12)
            x = 12;

        if (y + cardHeight > parentBounds.getBottom() - 12)
            y = localTargetRect.getY() - cardHeight - 6;
        if (y < 12)
            y = 12;

        setBounds(x, y, cardWidth, cardHeight);
        closeButton.setBounds(cardWidth - 28, 8, 20, 20);
        repaint();
    }

    void dismiss()
    {
        setVisible(false);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        // 1. Semi-transparent backdrop with drop shadow
        g.setColour(CamelPhatLookAndFeel::bgBase.withAlpha(0.96f));
        g.fillRoundedRectangle(bounds, 8.0f);

        // 2. Glowing Neon Border
        g.setColour(CamelPhatLookAndFeel::neonGreen.withAlpha(0.7f));
        g.drawRoundedRectangle(bounds, 8.0f, 1.2f);

        auto contentArea = bounds.reduced(14.0f);

        // 3. Header: Title
        auto titleArea = contentArea.removeFromTop(22.0f);
        titleArea.removeFromRight(24.0f); // Reserve space for close button
        g.setColour(CamelPhatLookAndFeel::neonGreen);
        g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        g.drawText(currentTopic.title, titleArea, juce::Justification::centredLeft, true);

        // 4. Signal Chain Role Pill
        if (currentTopic.signalRole.isNotEmpty())
        {
            auto roleArea = contentArea.removeFromTop(18.0f);
            g.setColour(CamelPhatLookAndFeel::neonCyan);
            g.setFont(juce::FontOptions(11.0f, juce::Font::plain));
            g.drawText(currentTopic.signalRole.toUpperCase(), roleArea, juce::Justification::centredLeft, true);
        }

        contentArea.removeFromTop(6.0f);

        // 5. Overview Paragraph
        auto overviewLayout = createOverviewLayout(currentTopic.overview, contentArea.getWidth());
        const float overviewHeight = std::ceil(overviewLayout.getHeight());
        auto overviewArea = contentArea.removeFromTop(overviewHeight);
        overviewLayout.draw(g, overviewArea);

        // Constant, uniform distance between description and divider/list
        constexpr float gapBetweenOverviewAndList = 10.0f;
        contentArea.removeFromTop(gapBetweenOverviewAndList);

        g.setColour(CamelPhatLookAndFeel::cardBorder);
        g.drawHorizontalLine(static_cast<int>(contentArea.getY()), bounds.getX() + 10.0f, bounds.getRight() - 10.0f);
        contentArea.removeFromTop(8.0f);

        // 6. Parameter Tips
        g.setFont(juce::FontOptions(11.5f, juce::Font::plain));
        constexpr float bulletSize = 4.5f;
        for (const auto& tip : currentTopic.parameterTips)
        {
            auto tipArea = contentArea.removeFromTop(24.0f);
            const float bulletX = tipArea.getX() + 2.0f;
            const float bulletY = tipArea.getY() + (tipArea.getHeight() - bulletSize) * 0.5f;

            // Bullet icon aligned to exact vertical center of text row
            g.setColour(CamelPhatLookAndFeel::neonAmber);
            g.fillEllipse(bulletX, bulletY, bulletSize, bulletSize);

            // Tip text
            g.setColour(CamelPhatLookAndFeel::textSecondary);
            g.drawText(tip, tipArea.withTrimmedLeft(14.0f).toNearestInt(), juce::Justification::centredLeft, true);
        }
    }

private:
    HelpTopic currentTopic;
    juce::TextButton closeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HelpOverlayComponent)
};

} // namespace camelphat::ui
