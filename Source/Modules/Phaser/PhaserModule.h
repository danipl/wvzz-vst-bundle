#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "PhaserDSP.h"
#include "PhaserView.h"

namespace wvzz::modules {

class PhaserModule : public IPluginModule
{
public:
    explicit PhaserModule(juce::String prefix = "phs")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~PhaserModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "phaser"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Phaser"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] PhaserDSP& getPhaserDSP() { return dsp; }
    [[nodiscard]] const PhaserDSP& getPhaserDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<PhaserView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    PhaserDSP dsp;
};

} // namespace wvzz::modules
