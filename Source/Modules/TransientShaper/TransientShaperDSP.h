#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>

namespace wvzz::modules {

/**
 * @brief Differential transient designer & inter-note noise gate.
 * Splits envelope into fast and slow follower curves to detect attack transients and sustain decay.
 */
class TransientShaperDSP : public IDSPModule
{
public:
    explicit TransientShaperDSP(juce::String prefix = "trans");
    ~TransientShaperDSP() override = default;

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

    void setParameters(float attackVal, float sustainVal, int speedMode = 0,
                       float gateThreshDb = -80.0f, bool bypass = false) noexcept;

private:
    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam  { nullptr };
    std::atomic<float>* attackParam  { nullptr };
    std::atomic<float>* sustainParam { nullptr };
    std::atomic<float>* speedParam   { nullptr };
    std::atomic<float>* gateParam    { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> attackSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sustainSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gateSmoothed;

    // Dual-envelope detector states
    float envFastL { 0.0f }, envFastR { 0.0f };
    float envSlowL { 0.0f }, envSlowR { 0.0f };

    // Dedicated stereo-linked noise gate state
    float gateEnv { 0.0f };
    float gateGain { 1.0f };
    int   gateHoldSamples { 0 };
    bool  gateIsOpen { true };
};

} // namespace wvzz::modules
