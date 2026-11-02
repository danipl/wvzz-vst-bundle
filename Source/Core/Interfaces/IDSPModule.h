#pragma once

#include "ProcessContext.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace wvzz {

/**
 * @brief Base contract for headless real-time audio DSP modules.
 * Adheres to Interface Segregation Principle (ISP) and Single Responsibility Principle (SRP).
 * Must have zero GUI dependencies and zero heap allocations / locks during processing.
 */
class IDSPModule
{
public:
    virtual ~IDSPModule() = default;

    /**
     * @brief Registers parameters into the plugin's APVTS ParameterLayout.
     */
    virtual void registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) = 0;

    /**
     * @brief Caches atomic float pointers from the constructed APVTS.
     */
    virtual void bindParameters(juce::AudioProcessorValueTreeState& apvts) = 0;

    /**
     * @brief Pre-allocates working buffers and configures sample rate and block size.
     */
    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;

    /**
     * @brief Resets filter states, delay lines, and envelopes to silence.
     */
    virtual void reset() noexcept = 0;

    /**
     * @brief Real-time audio processing callback.
     * Strictly zero-allocation, lock-free, and noexcept.
     */
    virtual void process(ProcessContext& ctx) noexcept = 0;

    /**
     * @brief Perturbs modulatable synthesis/effect parameters within musical bounds.
     */
    virtual void randomize(juce::Random& rng) = 0;
};

/**
 * @brief Segregated contract for modules providing audio level or gain reduction metering.
 */
class IMeterSource
{
public:
    virtual ~IMeterSource() = default;
    [[nodiscard]] virtual float getPeakLevel(int channel) const noexcept { juce::ignoreUnused(channel); return 0.0f; }
    [[nodiscard]] virtual float getRmsLevel(int channel) const noexcept { juce::ignoreUnused(channel); return 0.0f; }
    [[nodiscard]] virtual float getGainReduction() const noexcept { return 0.0f; }
};

} // namespace wvzz
