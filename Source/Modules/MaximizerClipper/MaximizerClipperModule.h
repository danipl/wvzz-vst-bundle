#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "MaximizerClipperDSP.h"
#include "MaximizerClipperView.h"

namespace wvzz::modules {

class MaximizerClipperModule : public IPluginModule
{
public:
    explicit MaximizerClipperModule(juce::String prefix = "clip")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~MaximizerClipperModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "maximizer_clipper"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Maximizer Clipper"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] MaximizerClipperDSP& getClipperDSP() { return dsp; }
    [[nodiscard]] const MaximizerClipperDSP& getClipperDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<MaximizerClipperView>(apvts, instancePrefix, &dsp, helpHost);
    }

private:
    juce::String instancePrefix;
    MaximizerClipperDSP dsp;
};

} // namespace wvzz::modules
