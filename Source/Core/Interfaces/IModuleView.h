#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace wvzz::ui {

/**
 * @brief Base component interface for self-contained module UI cards.
 * Encapsulates card chrome, header controls, local layout, and periodic visual updates.
 */
class IModuleView : public juce::Component
{
public:
    ~IModuleView() override = default;

    /**
     * @brief Periodic visual update hook called by the host editor's 60 FPS timer.
     */
    virtual void updateVisuals() {}

    /**
     * @brief Returns inner content area for controls (excluding header bar).
     */
    [[nodiscard]] virtual juce::Rectangle<int> getContentBounds() const noexcept
    {
        return getLocalBounds().reduced(8).withTrimmedTop(32);
    }
};

} // namespace wvzz::ui
