#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "FlangerDSP.h"
#include "FlangerView.h"

namespace wvzz::modules {

class FlangerModule : public IPluginModule
{
public:
    /** @param extendedControls true shows Feedback, Depth, Center, and Through-Zero in the view. */
    explicit FlangerModule(juce::String prefix = "flg", bool extendedControls = false)
        : instancePrefix(std::move(prefix)),
          extendedView(extendedControls),
          dsp(instancePrefix)
    {
    }

    ~FlangerModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "flanger"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Flanger"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] FlangerDSP& getFlangerDSP() { return dsp; }
    [[nodiscard]] const FlangerDSP& getFlangerDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<FlangerView>(apvts, instancePrefix, helpHost, extendedView);
    }

private:
    juce::String instancePrefix;
    bool extendedView { false };
    FlangerDSP dsp;
};

} // namespace wvzz::modules
