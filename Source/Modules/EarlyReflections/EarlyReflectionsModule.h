#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "EarlyReflectionsDSP.h"
#include "EarlyReflectionsView.h"

namespace wvzz::modules {

class EarlyReflectionsModule : public IPluginModule
{
public:
    explicit EarlyReflectionsModule(juce::String prefix = "er")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~EarlyReflectionsModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "early_reflections"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Early Reflections"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] EarlyReflectionsDSP& getEarlyReflectionsDSP() { return dsp; }
    [[nodiscard]] const EarlyReflectionsDSP& getEarlyReflectionsDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<EarlyReflectionsView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    EarlyReflectionsDSP dsp;
};

} // namespace wvzz::modules
