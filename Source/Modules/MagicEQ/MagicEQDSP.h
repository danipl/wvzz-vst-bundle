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

    void setParameters(float amountNorm, float tuneHz, bool phatModeEnabled, bool bypass = false) noexcept;

private:
    void updateCoefficients(float amountVal, float tuneVal) noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam { nullptr };
    std::atomic<float>* amountParam { nullptr };
    std::atomic<float>* tuneParam   { nullptr };
    std::atomic<float>* phatParam   { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amountSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tuneSmoothed;

    float b0_boost { 1.0f }, b1_boost { 0.0f }, b2_boost { 0.0f };
    float a1_boost { 0.0f }, a2_boost { 0.0f };
    float boostS1L { 0.0f }, boostS2L { 0.0f };
    float boostS1R { 0.0f }, boostS2R { 0.0f };

    float b0_dip { 1.0f }, b1_dip { 0.0f }, b2_dip { 0.0f };
    float a1_dip { 0.0f }, a2_dip { 0.0f };
    float dipS1L { 0.0f }, dipS2L { 0.0f };
    float dipS1R { 0.0f }, dipS2R { 0.0f };

    juce::dsp::IIR::Filter<float> highShelfL;
    juce::dsp::IIR::Filter<float> highShelfR;
};

} // namespace wvzz::modules
