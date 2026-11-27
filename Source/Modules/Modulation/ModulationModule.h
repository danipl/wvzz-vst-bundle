#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "ModulationDSP.h"
#include "ModulationView.h"

namespace wvzz::modules {

class ModulationModule : public IPluginModule
{
public:
    explicit ModulationModule(juce::String prefix = "mod", dsp::AudioVisualizerSource* visualizer = nullptr, ModProfile profile = ModProfile::Distortion)
        : instancePrefix(std::move(prefix)),
          visualizerSource(visualizer),
          dsp(instancePrefix, profile)
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

    /**
     * @brief Registers one plugin-specific display, shown as a third tab in the center column.
     * The factory runs on the message thread each time a view is created.
     */
    void setCustomDisplay(juce::String tabTitle, ModulationView::CustomDisplayFactory factory)
    {
        customDisplayTitle = std::move(tabTitle);
        customDisplayFactory = std::move(factory);
    }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        auto view = std::make_unique<ModulationView>(apvts, dsp, visualizerSource, helpHost, dsp.getModProfile());
        if (customDisplayFactory)
            view->setCustomDisplay(customDisplayTitle, customDisplayFactory());
        return view;
    }

private:
    juce::String instancePrefix;
    dsp::AudioVisualizerSource* visualizerSource;
    ModulationDSP dsp;
    juce::String customDisplayTitle;
    ModulationView::CustomDisplayFactory customDisplayFactory;
};

} // namespace wvzz::modules
