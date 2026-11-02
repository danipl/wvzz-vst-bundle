#pragma once

#include "IDSPModule.h"
#include "IModuleView.h"
#include <juce_core/juce_core.h>
#include <memory>

namespace wvzz::ui {
class IHelpOverlayHost;
}

namespace wvzz {

/**
 * @brief High-level module contract binding headless DSP and on-demand UI view factory.
 * Adheres to SOLID principles (SRP, OCP, LSP, ISP, DIP).
 */
class IPluginModule
{
public:
    virtual ~IPluginModule() = default;

    [[nodiscard]] virtual juce::String getModuleId() const = 0;
    [[nodiscard]] virtual juce::String getDisplayName() const = 0;
    [[nodiscard]] virtual juce::String getInstancePrefix() const = 0;

    [[nodiscard]] virtual IDSPModule& getDSP() = 0;
    [[nodiscard]] virtual const IDSPModule& getDSP() const = 0;

    /**
     * @brief Factory creating the UI view for this module.
     * Decouples headless DSP from GUI instantiation.
     */
    [[nodiscard]] virtual std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) = 0;
};

} // namespace wvzz
