#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "DelayDSP.h"
#include "DelayView.h"

namespace wvzz::modules {

class DelayModule : public IPluginModule
{
public:
    explicit DelayModule(juce::String prefix = "dly")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~DelayModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "delay"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Delay"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] DelayDSP& getDelayDSP() { return dsp; }
    [[nodiscard]] const DelayDSP& getDelayDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<DelayView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    DelayDSP dsp;
};

} // namespace wvzz::modules
