#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "TransientShaperDSP.h"
#include "TransientShaperView.h"

namespace wvzz::modules {

class TransientShaperModule : public IPluginModule
{
public:
    explicit TransientShaperModule(juce::String prefix = "trans")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~TransientShaperModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "transient_shaper"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Transient & Gate"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] TransientShaperDSP& getTransientDSP() { return dsp; }
    [[nodiscard]] const TransientShaperDSP& getTransientDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<TransientShaperView>(apvts, instancePrefix, &dsp, helpHost);
    }

private:
    juce::String instancePrefix;
    TransientShaperDSP dsp;
};

} // namespace wvzz::modules
