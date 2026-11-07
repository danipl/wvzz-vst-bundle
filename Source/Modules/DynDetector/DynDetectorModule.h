#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "DynDetectorDSP.h"
#include "DynDetectorView.h"

namespace wvzz::modules {

class DynDetectorModule : public IPluginModule
{
public:
    explicit DynDetectorModule(juce::String prefix = "det")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~DynDetectorModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "dyn_detector"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Sub & Detector"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] DynDetectorDSP& getDetectorDSP() { return dsp; }
    [[nodiscard]] const DynDetectorDSP& getDetectorDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<DynDetectorView>(apvts, instancePrefix, &dsp, helpHost);
    }

private:
    juce::String instancePrefix;
    DynDetectorDSP dsp;
};

} // namespace wvzz::modules
