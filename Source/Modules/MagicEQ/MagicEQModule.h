#pragma once

#include "../../Core/Interfaces/IPluginModule.h"
#include "MagicEQDSP.h"
#include "MagicEQView.h"

namespace wvzz::modules {

class MagicEQModule : public IPluginModule
{
public:
    explicit MagicEQModule(juce::String prefix = "meq")
        : instancePrefix(std::move(prefix)),
          dsp(instancePrefix)
    {
    }

    ~MagicEQModule() override = default;

    [[nodiscard]] juce::String getModuleId() const override { return "magic_eq"; }
    [[nodiscard]] juce::String getDisplayName() const override { return "Magic EQ"; }
    [[nodiscard]] juce::String getInstancePrefix() const override { return instancePrefix; }

    [[nodiscard]] IDSPModule& getDSP() override { return dsp; }
    [[nodiscard]] const IDSPModule& getDSP() const override { return dsp; }

    [[nodiscard]] MagicEQDSP& getMagicEQDSP() { return dsp; }
    [[nodiscard]] const MagicEQDSP& getMagicEQDSP() const { return dsp; }

    [[nodiscard]] std::unique_ptr<ui::IModuleView> createView(
        juce::AudioProcessorValueTreeState& apvts,
        ui::IHelpOverlayHost* helpHost = nullptr) override
    {
        return std::make_unique<MagicEQView>(apvts, instancePrefix, helpHost);
    }

private:
    juce::String instancePrefix;
    MagicEQDSP dsp;
};

} // namespace wvzz::modules
