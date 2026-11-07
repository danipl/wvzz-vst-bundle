#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace wvzz::modules {

class MagicEQDSP : public IDSPModule
{
public:
    explicit MagicEQDSP(juce::String prefix = "meq");
    ~MagicEQDSP() override = default;

    [[nodiscard]] juce::String paramId(const juce::String& name) const
    {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    }

    void registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    void bindParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() noexcept override;
    void process(ProcessContext& ctx) noexcept override;
    void randomize(juce::Random& rng) override;

    [[nodiscard]] bool isPreDistortion() const noexcept
    {
        return preParam != nullptr && preParam->load(std::memory_order_relaxed) > 0.5f;
    }

    void setParameters(float amountNorm, float tuneHz, bool phatModeEnabled, bool bypass = false, bool pre = false) noexcept;

private:
    void updateCoefficients(float amountVal, float tuneVal) noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam { nullptr };
    std::atomic<float>* amountParam { nullptr };
    std::atomic<float>* tuneParam   { nullptr };
    std::atomic<float>* phatParam   { nullptr };
    std::atomic<float>* preParam    { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amountSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tuneSmoothed;

    double b0_boost { 1.0 }, b1_boost { 0.0 }, b2_boost { 0.0 };
    double a1_boost { 0.0 }, a2_boost { 0.0 };
    double boostS1L { 0.0 }, boostS2L { 0.0 };
    double boostS1R { 0.0 }, boostS2R { 0.0 };

    double b0_dip { 1.0 }, b1_dip { 0.0 }, b2_dip { 0.0 };
    double a1_dip { 0.0 }, a2_dip { 0.0 };
    double dipS1L { 0.0 }, dipS2L { 0.0 };
    double dipS1R { 0.0 }, dipS2R { 0.0 };

    double b0_shelf { 1.0 }, b1_shelf { 0.0 }, b2_shelf { 0.0 };
    double a1_shelf { 0.0 }, a2_shelf { 0.0 };
    double shelfS1L { 0.0 }, shelfS2L { 0.0 };
    double shelfS1R { 0.0 }, shelfS2R { 0.0 };
};

} // namespace wvzz::modules
