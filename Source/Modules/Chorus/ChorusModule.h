#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "ChorusDSP.h"
#include "ChorusView.h"

namespace wvzz::modules {

class ChorusModule : public IPluginModule
{
public:
    explicit ChorusModule(juce::String prefix = "chor")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~ChorusModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "chorus"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Chorus"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] ChorusDSP& getChorusDSP() { return dsp; }
    [[nodiscard]] const ChorusDSP& getChorusDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<ChorusView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    ChorusDSP dsp;
};

} // namespace wvzz::modules
