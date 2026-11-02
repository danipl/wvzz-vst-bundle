#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "CompressorDSP.h"
#include "CompressorView.h"

namespace wvzz::modules {

class CompressorModule : public IPluginModule
{
public:
    explicit CompressorModule(juce::String prefix = "comp")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~CompressorModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "compressor"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Compressor"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] CompressorDSP& getCompressorDSP() { return dsp; }
    [[nodiscard]] const CompressorDSP& getCompressorDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<CompressorView>(apvts, instancePrefix, &dsp, helpHost);
    }

private:
    juce::String instancePrefix;
    CompressorDSP dsp;
};

} // namespace wvzz::modules
