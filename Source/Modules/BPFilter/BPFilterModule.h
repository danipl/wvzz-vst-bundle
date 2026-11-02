#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "BPFilterDSP.h"
#include "BPFilterView.h"

namespace wvzz::modules {

class BPFilterModule : public IPluginModule
{
public:
    explicit BPFilterModule(juce::String prefix = "bp")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~BPFilterModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "bp_filter"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "BP Filter"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] BPFilterDSP& getBPFilterDSP() { return dsp; }
    [[nodiscard]] const BPFilterDSP& getBPFilterDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<BPFilterView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    BPFilterDSP dsp;
};

} // namespace wvzz::modules
