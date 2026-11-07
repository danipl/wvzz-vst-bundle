#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "RhythmicPumperDSP.h"
#include "RhythmicPumperView.h"

namespace wvzz::modules {

class RhythmicPumperModule : public IPluginModule
{
public:
    explicit RhythmicPumperModule(juce::String prefix = "pump")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~RhythmicPumperModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "rhythmic_pumper"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Rhythmic Pumper"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] RhythmicPumperDSP& getPumperDSP() { return dsp; }
    [[nodiscard]] const RhythmicPumperDSP& getPumperDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<RhythmicPumperView>(apvts, instancePrefix, &dsp, helpHost);
    }

private:
    juce::String instancePrefix;
    RhythmicPumperDSP dsp;
};

} // namespace wvzz::modules
