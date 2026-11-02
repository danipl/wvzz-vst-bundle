#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "MMFilterDSP.h"
#include "MMFilterView.h"

namespace wvzz::modules {

class MMFilterModule : public IPluginModule
{
public:
    explicit MMFilterModule(juce::String prefix = "mm")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~MMFilterModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "mm_filter"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "MM Filter"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] MMFilterDSP& getMMFilterDSP() { return dsp; }
    [[nodiscard]] const MMFilterDSP& getMMFilterDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<MMFilterView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    MMFilterDSP dsp;
};

} // namespace wvzz::modules
