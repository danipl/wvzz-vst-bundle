#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "MultibandOTTDSP.h"
#include "MultibandOTTView.h"

namespace wvzz::modules {

class MultibandOTTModule : public IPluginModule
{
public:
    explicit MultibandOTTModule(juce::String prefix = "ott")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~MultibandOTTModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "multiband_ott"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Hyper-OTT"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] MultibandOTTDSP& getOTTDSP() { return dsp; }
    [[nodiscard]] const MultibandOTTDSP& getOTTDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<MultibandOTTView>(apvts, instancePrefix, &dsp, helpHost);
    }

private:
    juce::String instancePrefix;
    MultibandOTTDSP dsp;
};

} // namespace wvzz::modules
