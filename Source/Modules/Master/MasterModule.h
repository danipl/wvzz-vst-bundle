#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "MasterDSP.h"
#include "MasterView.h"

namespace wvzz::modules {

class MasterModule : public IPluginModule
{
public:
    explicit MasterModule(juce::String prefix = "master")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~MasterModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "master"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Master"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] MasterDSP& getMasterDSP() { return dsp; }
    [[nodiscard]] const MasterDSP& getMasterDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<MasterView>(apvts, instancePrefix, &dsp, helpHost);
    }

private:
    juce::String instancePrefix;
    MasterDSP dsp;
};

} // namespace wvzz::modules
