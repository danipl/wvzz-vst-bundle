#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "TremoloDSP.h"
#include "TremoloView.h"

namespace wvzz::modules {

class TremoloModule : public IPluginModule
{
public:
    explicit TremoloModule(juce::String prefix = "trem")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~TremoloModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "tremolo"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Tremolo"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] TremoloDSP& getTremoloDSP() { return dsp; }
    [[nodiscard]] const TremoloDSP& getTremoloDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<TremoloView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    TremoloDSP dsp;
};

} // namespace wvzz::modules
