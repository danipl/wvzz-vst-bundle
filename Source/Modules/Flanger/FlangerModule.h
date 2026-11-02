#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "FlangerDSP.h"
#include "FlangerView.h"

namespace wvzz::modules {

class FlangerModule : public IPluginModule
{
public:
    explicit FlangerModule(juce::String prefix = "flg")
        : instancePrefix(std::move(prefix)),
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
        return std::make_unique<FlangerView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    FlangerDSP dsp;
};

} // namespace wvzz::modules
