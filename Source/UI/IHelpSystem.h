#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <vector>

namespace camelphat::ui {

/**
 * @brief Structured help content payload describing a component or module.
 */
struct HelpTopic
{
    juce::String title;
    juce::String overview;
    juce::String signalRole;
    std::vector<juce::String> parameterTips;
};

/**
 * @brief Interface contract for querying contextual help content.
 * Adheres to Interface Segregation Principle (ISP) and Single Responsibility Principle (SRP).
 */
class IHelpContentProvider
{
public:
    virtual ~IHelpContentProvider() = default;

    /**
     * @brief Fetches contextual explanation metadata for a given topic/module ID.
     */
    [[nodiscard]] virtual HelpTopic getTopic(const juce::String& topicId) const = 0;
};

/**
 * @brief Interface contract allowing child UI widgets to trigger floating help popovers.
 * Decouples child panels from top-level editor layout (Dependency Inversion Principle).
 */
class IHelpOverlayHost
{
public:
    virtual ~IHelpOverlayHost() = default;

    /**
     * @brief Displays the contextual help overlay anchored to the triggering button.
     * @param topicId The unique topic identifier.
     * @param sourceBounds The screen coordinates of the triggering "?" button.
     */
    virtual void showHelp(const juce::String& topicId, juce::Rectangle<int> sourceBounds) = 0;

    /**
     * @brief Dismisses the active help overlay.
     */
    virtual void hideHelp() = 0;

    /**
     * @brief Returns whether a help card is currently displayed.
     */
    [[nodiscard]] virtual bool isHelpVisible() const noexcept = 0;
};

} // namespace camelphat::ui
