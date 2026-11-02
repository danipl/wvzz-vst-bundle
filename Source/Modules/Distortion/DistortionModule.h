#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "DistortionDSP.h"
#include "DistortionView.h"

namespace wvzz::modules {

class DistortionModule : public IPluginModule
{
public:
    explicit DistortionModule(juce::String prefix = "dist")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~DistortionModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "distortion"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Distortion"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] DistortionDSP& getDistortionDSP() { return dsp; }
    [[nodiscard]] const DistortionDSP& getDistortionDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<DistortionView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    DistortionDSP dsp;
};

} // namespace wvzz::modules
