#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "IHelpSystem.h"
#include "CamelPhatLookAndFeel.h"

namespace camelphat::ui {

/**
 * @brief Interactive vector button with a "?" symbol to trigger contextual explanations.
 */
class HelpButtonComponent : public juce::Component,
                             public juce::SettableTooltipClient
{
public:
    HelpButtonComponent(juce::String id, IHelpOverlayHost& overlayHost)
        : topicId(std::move(id)), host(overlayHost)
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        setTooltip("Click for module explanation and tips");
    }

    ~HelpButtonComponent() override = default;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.5f);

        const bool isHover = isMouseOver();
        const auto circleColor = isHover ? CamelPhatLookAndFeel::neonGreen : CamelPhatLookAndFeel::cardBorder;
        const auto textColor   = isHover ? CamelPhatLookAndFeel::neonGreen : CamelPhatLookAndFeel::textSecondary;

        // Circular outline
        g.setColour(circleColor);
        g.drawEllipse(bounds, 1.2f);

        if (isHover)
        {
            g.setColour(CamelPhatLookAndFeel::neonGreen.withAlpha(0.12f));
            g.fillEllipse(bounds);
        }

        // "?" Glyph
        g.setColour(textColor);
        g.setFont(juce::FontOptions(bounds.getHeight() * 0.65f, juce::Font::bold));
        g.drawText("?", getLocalBounds(), juce::Justification::centred, false);
    }

    void mouseEnter(const juce::MouseEvent&) override { repaint(); }
    void mouseExit(const juce::MouseEvent&) override  { repaint(); }

    void mouseDown(const juce::MouseEvent&) override
    {
        if (host.isHelpVisible())
            host.hideHelp();
        else
            host.showHelp(topicId, getScreenBounds());
    }

private:
    juce::String topicId;
    IHelpOverlayHost& host;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HelpButtonComponent)
};

} // namespace camelphat::ui
