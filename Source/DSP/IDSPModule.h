#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace camelphat::dsp {

/**
 * @brief Base contract for any audio DSP processing module.
 * Adheres to Interface Segregation Principle (ISP) and Single Responsibility Principle (SRP).
 */
class IDSPModule
{
public:
    virtual ~IDSPModule() = default;

    /**
     * @brief Pre-allocates working buffers and configures sample rate and block size.
     * Guaranteed to be called on message thread prior to real-time audio processing.
     */
    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;

    /**
     * @brief Resets filter states, delay lines, and envelopes to silence.
     * Must be real-time safe and strictly noexcept.
     */
    virtual void reset() noexcept = 0;
};

/**
 * @brief Contract for modules that transform stereo audio buffers in-place.
 */
class IAudioProcessorModule : public IDSPModule
{
public:
    /**
     * @brief Real-time audio processing callback.
     * Must be strictly zero-allocation, lock-free, and noexcept.
     */
    virtual void process(juce::AudioBuffer<float>& buffer) noexcept = 0;
};

/**
 * @brief Contract for modules providing atomic audio level metering to the UI thread.
 */
class IMeterSource
{
public:
    virtual ~IMeterSource() = default;
    [[nodiscard]] virtual float getPeakLevel(int channel) const noexcept = 0;
    [[nodiscard]] virtual float getRmsLevel(int channel) const noexcept = 0;
};

} // namespace camelphat::dsp
