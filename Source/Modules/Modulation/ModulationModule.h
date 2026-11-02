#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "ModulationDSP.h"
#include "ModulationView.h"

namespace wvzz::modules {

class ModulationModule : public IPluginModule
{
public:
    explicit ModulationModule(juce::String prefix = "mod", dsp::AudioVisualizerSource* visualizer = nullptr)
        : instancePrefix(std::move(prefix)),
          visualizerSource(visualizer),
          dsp(instancePrefix)
    {
    }

    ~ModulationModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "modulation"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Modulation"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] ModulationDSP& getModulationDSP() { return dsp; }
    [[nodiscard]] const ModulationDSP& getModulationDSP() const { return dsp; }

    void setVisualizerSource(dsp::AudioVisualizerSource* visualizer) { visualizerSource = visualizer; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<ModulationView>(apvts, dsp, visualizerSource, helpHost);
    }

private:
    juce::String instancePrefix;
    dsp::AudioVisualizerSource* visualizerSource;
    ModulationDSP dsp;
};

} // namespace wvzz::modules
