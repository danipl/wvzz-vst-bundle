#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "GrainDSP.h"
#include "GrainView.h"

namespace wvzz::modules {

class GrainModule : public IPluginModule
{
public:
    explicit GrainModule(juce::String prefix = "grain")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~GrainModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "grain"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Grain"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] GrainDSP& getGrainDSP() { return dsp; }
    [[nodiscard]] const GrainDSP& getGrainDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<GrainView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    GrainDSP dsp;
};

} // namespace wvzz::modules
