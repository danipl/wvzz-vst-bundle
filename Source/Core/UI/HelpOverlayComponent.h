#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "IHelpSystem.h"
#include "WvZzLookAndFeel.h"

namespace wvzz::ui {

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
        closeButton.setColour(juce::TextButton::textColourOffId, WvZzLookAndFeel::textSecondary);
        addAndMakeVisible(closeButton);
    }

    ~HelpOverlayComponent() override = default;

    static juce::TextLayout createOverviewLayout(const juce::String& text, float maxWidth)
    {
        juce::AttributedString attr;
        attr.append(text, juce::FontOptions(11.0f, juce::Font::plain), WvZzLookAndFeel::textPrimary);
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
        const float contentWidth = static_cast<float>(cardWidth - 26);

        // Measure dynamic text layout height for description
        auto overviewLayout = createOverviewLayout(topic.overview, contentWidth);
        const float overviewH = std::ceil(overviewLayout.getHeight());

        // Header: 32 + Margins: 16 (8 top, 8 bot)
        int baseHeight = 32 + 16;
        if (topic.signalRole.isNotEmpty())
            baseHeight += 24; // 18 tag + 6 gap

        constexpr float gapBetweenOverviewAndList = 10.0f;
        const int tipsCount = static_cast<int>(topic.parameterTips.size());
        constexpr int tipRowHeight = 24;

        const int cardHeight = baseHeight
                             + static_cast<int>(overviewH)
                             + static_cast<int>(gapBetweenOverviewAndList)
                             + (tipsCount * tipRowHeight)
                             + 8; // Bottom breathing room

        auto parentBounds = parentContainer->getLocalBounds();
        // Convert screen target bounds into parent container's local coordinates
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

        // 1. Drop shadow & card background
        g.setColour(juce::Colour(0xff0e1318));
        g.fillRoundedRectangle(bounds, 8.0f);

        g.setColour(WvZzLookAndFeel::neonGreen.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 8.0f, 1.2f);

        // 2. Title Bar
        auto headerArea = bounds.removeFromTop(32.0f);
        headerArea.reduce(12.0f, 0.0f);

        g.setColour(WvZzLookAndFeel::neonGreen);
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(currentTopic.title, headerArea.withTrimmedRight(24.0f), juce::Justification::centredLeft, true);

        // Divider
        g.setColour(WvZzLookAndFeel::cardBorder);
        g.drawHorizontalLine(32, bounds.getX(), bounds.getRight());

        // 3. Role / Signal Path Tag
        auto contentArea = bounds.reduced(12.0f, 8.0f);
        if (currentTopic.signalRole.isNotEmpty())
        {
            auto tagArea = contentArea.removeFromTop(18.0f);
            auto tagFont = juce::FontOptions(10.0f, juce::Font::bold);
            g.setFont(tagFont);
            const float textW = juce::Font(tagFont).getStringWidthFloat(currentTopic.signalRole) + 16.0f;
            g.setColour(WvZzLookAndFeel::neonAmber.withAlpha(0.15f));
            g.fillRoundedRectangle(tagArea.withWidth(textW), 3.0f);
            g.setColour(WvZzLookAndFeel::neonAmber);
            g.drawText(currentTopic.signalRole, tagArea.withTrimmedLeft(6.0f), juce::Justification::centredLeft, false);
            contentArea.removeFromTop(6.0f);
        }

        // 4. Overview Body Text
        auto overviewLayout = createOverviewLayout(currentTopic.overview, contentArea.getWidth());
        const float overviewHeight = std::ceil(overviewLayout.getHeight());
        auto overviewArea = contentArea.removeFromTop(overviewHeight);
        overviewLayout.draw(g, overviewArea);

        // Constant, uniform distance between description and parameter tips list
        constexpr float gapBetweenOverviewAndList = 10.0f;
        contentArea.removeFromTop(gapBetweenOverviewAndList);

        // 5. Parameter Tips Bullet List
        constexpr float bulletSize = 4.0f;
        for (const auto& tip : currentTopic.parameterTips)
        {
            auto tipArea = contentArea.removeFromTop(24.0f);

            // Bullet icon aligned to exact vertical center of text row
            const float bulletX = tipArea.getX() + 2.0f;
            const float bulletY = tipArea.getY() + (tipArea.getHeight() - bulletSize) * 0.5f;

            g.setColour(WvZzLookAndFeel::neonCyan);
            g.fillEllipse(bulletX, bulletY, bulletSize, bulletSize);

            // Tip text
            g.setColour(WvZzLookAndFeel::textSecondary);
            g.setFont(juce::FontOptions(10.5f, juce::Font::plain));
            g.drawFittedText(tip, tipArea.withTrimmedLeft(14.0f).toNearestInt(), juce::Justification::centredLeft, 1);
        }
    }

private:
    HelpTopic currentTopic;
    juce::TextButton closeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HelpOverlayComponent)
};

} // namespace wvzz::ui
