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

    void displayTopic(const HelpTopic& topic, juce::Rectangle<int> targetScreenBounds, juce::Component* parentContainer)
    {
        currentTopic = topic;
        setVisible(true);

        const int cardWidth = 340;
        const int cardHeight = 160 + static_cast<int>(topic.parameterTips.size()) * 26;

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
            g.setColour(WvZzLookAndFeel::neonAmber.withAlpha(0.15f));
            g.fillRoundedRectangle(tagArea.withWidth(g.getCurrentFont().getStringWidth(currentTopic.signalRole) + 16.0f), 3.0f);
            g.setColour(WvZzLookAndFeel::neonAmber);
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.drawText(currentTopic.signalRole, tagArea.withTrimmedLeft(6.0f), juce::Justification::centredLeft, false);
            contentArea.removeFromTop(6.0f);
        }

        // 4. Overview Body Text
        g.setColour(WvZzLookAndFeel::textPrimary);
        g.setFont(juce::FontOptions(11.0f, juce::Font::plain));
        auto overviewArea = contentArea.removeFromTop(44.0f);
        g.drawFittedText(currentTopic.overview, overviewArea.toNearestInt(), juce::Justification::topLeft, 3);

        // 5. Parameter Tips Bullet List
        contentArea.removeFromTop(4.0f);
        for (const auto& tip : currentTopic.parameterTips)
        {
            auto tipArea = contentArea.removeFromTop(24.0f);

            // Bullet icon
            g.setColour(WvZzLookAndFeel::neonCyan);
            g.fillEllipse(tipArea.getX() + 2.0f, tipArea.getY() + 6.0f, 4.0f, 4.0f);

            // Tip text
            g.setColour(WvZzLookAndFeel::textSecondary);
            g.setFont(juce::FontOptions(10.5f, juce::Font::plain));
            g.drawFittedText(tip, tipArea.withTrimmedLeft(12.0f).toNearestInt(), juce::Justification::centredLeft, 1);
        }
    }

private:
    HelpTopic currentTopic;
    juce::TextButton closeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HelpOverlayComponent)
};

} // namespace wvzz::ui
