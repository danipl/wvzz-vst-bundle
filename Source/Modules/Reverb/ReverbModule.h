#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "ReverbDSP.h"
#include "ReverbView.h"

namespace wvzz::modules {

class ReverbModule : public IPluginModule
{
public:
    explicit ReverbModule(juce::String prefix = "rev")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~ReverbModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "reverb"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Reverb"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] ReverbDSP& getReverbDSP() { return dsp; }
    [[nodiscard]] const ReverbDSP& getReverbDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<ReverbView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    ReverbDSP dsp;
};

} // namespace wvzz::modules
