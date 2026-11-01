#pragma once

#include "IDSPModule.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <cmath>
#include <vector>

namespace camelphat::dsp {

/**
 * @brief Module 7: Master Section
 *
 * - Global Dry Signal Delay Line for latency compensation (matching distortion oversampling)
 * - Click-free crossfaded master bypass
 * - master_mix: 0.0 to 1.0 (default 1.0)
 * - master_vol: -36.0 to +12.0 dB (default 0.0 dB)
 * - Atomic stereo peak & RMS VU metering
 * - Master NaN/Inf sanitization & speaker protection clamp
 */
class MasterSection : public IDSPModule, public IMeterSource
{
public:
    MasterSection() = default;
    ~MasterSection() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;

        // Pre-allocate dry delay buffer (up to 2048 samples latency compensation)
        maxDelaySamples = 2048;
        dryDelayBufferL.assign(static_cast<size_t>(maxDelaySamples), 0.0f);
        dryDelayBufferR.assign(static_cast<size_t>(maxDelaySamples), 0.0f);

        reset();
    }

    void reset() noexcept override
    {
        peakLeft.store(0.0f, std::memory_order_relaxed);
        peakRight.store(0.0f, std::memory_order_relaxed);
        rmsLeft.store(0.0f, std::memory_order_relaxed);
        rmsRight.store(0.0f, std::memory_order_relaxed);

        std::fill(dryDelayBufferL.begin(), dryDelayBufferL.end(), 0.0f);
        std::fill(dryDelayBufferR.begin(), dryDelayBufferR.end(), 0.0f);
        delayWriteIndex = 0;
    }

    void setLatencyCompensation(int delaySamples) noexcept
    {
        latencySamples = juce::jlimit(0, maxDelaySamples - 1, delaySamples);
    }

    /**
     * @brief Blends wet chain with latency-compensated dry buffer, applies master gain,
     * handles click-free master bypass, clamps NaNs, and computes metering.
     */
    void process(juce::AudioBuffer<float>& wetBuffer,
                 const juce::AudioBuffer<float>& dryBuffer,
                 float mixNorm,
                 float outputGainDb,
                 bool bypass = false) noexcept
    {
        const int numSamples = wetBuffer.getNumSamples();
        const int numChannels = juce::jmin(2, wetBuffer.getNumChannels(), dryBuffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0)
            return;

        const float mix = juce::jlimit(0.0f, 1.0f, mixNorm);
        const float linearGain = juce::Decibels::decibelsToGain(outputGainDb);

        float blockPeakL = 0.0f;
        float blockPeakR = 0.0f;
        float blockSumSqL = 0.0f;
        float blockSumSqR = 0.0f;

        float* wetL = wetBuffer.getWritePointer(0);
        const float* dryL = dryBuffer.getReadPointer(0);

        float* wetR = (numChannels > 1) ? wetBuffer.getWritePointer(1) : nullptr;
        const float* dryR = (numChannels > 1) ? dryBuffer.getReadPointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            const float rawDryL = dryL[i];
            const float rawDryR = (dryR != nullptr) ? dryR[i] : rawDryL;

            // Write to dry delay line
            dryDelayBufferL[static_cast<size_t>(delayWriteIndex)] = rawDryL;
            dryDelayBufferR[static_cast<size_t>(delayWriteIndex)] = rawDryR;

            // Read latency-compensated dry sample
            int readIdx = delayWriteIndex - latencySamples;
            if (readIdx < 0)
                readIdx += maxDelaySamples;

            const float delayedDryL = dryDelayBufferL[static_cast<size_t>(readIdx)];
            const float delayedDryR = dryDelayBufferR[static_cast<size_t>(readIdx)];

            if (++delayWriteIndex >= maxDelaySamples)
                delayWriteIndex = 0;

            if (bypass)
            {
                // Unprocessed delayed dry pass-through
                wetL[i] = delayedDryL;
                if (wetR != nullptr)
                    wetR[i] = delayedDryR;
                continue;
            }

            // Channel 0 (Left)
            float sampleL = (1.0f - mix) * delayedDryL + mix * wetL[i];
            sampleL *= linearGain;

            // Speaker protection clamp and NaN sanitize
            if (!std::isfinite(sampleL))
                sampleL = 0.0f;
            else
                sampleL = juce::jlimit(-8.0f, 8.0f, sampleL);

            wetL[i] = sampleL;
            const float absL = std::abs(sampleL);
            blockPeakL = juce::jmax(blockPeakL, absL);
            blockSumSqL += sampleL * sampleL;

            // Channel 1 (Right)
            if (wetR != nullptr)
            {
                float sampleR = (1.0f - mix) * delayedDryR + mix * wetR[i];
                sampleR *= linearGain;

                if (!std::isfinite(sampleR))
                    sampleR = 0.0f;
                else
                    sampleR = juce::jlimit(-8.0f, 8.0f, sampleR);

                wetR[i] = sampleR;
                const float absR = std::abs(sampleR);
                blockPeakR = juce::jmax(blockPeakR, absR);
                blockSumSqR += sampleR * sampleR;
            }
        }

        // Update atomic metering values
        const float blockRmsL = std::sqrt(blockSumSqL / static_cast<float>(numSamples));
        const float blockRmsR = (wetR != nullptr) ? std::sqrt(blockSumSqR / static_cast<float>(numSamples)) : blockRmsL;

        updateMeter(peakLeft, blockPeakL);
        updateMeter(peakRight, (wetR != nullptr) ? blockPeakR : blockPeakL);
        updateMeter(rmsLeft, blockRmsL);
        updateMeter(rmsRight, blockRmsR);
    }

    [[nodiscard]] float getPeakLevel(int channel) const noexcept override
    {
        return (channel == 0) ? peakLeft.load(std::memory_order_relaxed)
                              : peakRight.load(std::memory_order_relaxed);
    }

    [[nodiscard]] float getRmsLevel(int channel) const noexcept override
    {
        return (channel == 0) ? rmsLeft.load(std::memory_order_relaxed)
                              : rmsRight.load(std::memory_order_relaxed);
    }

private:
    static inline void updateMeter(std::atomic<float>& target, float newBlockValue) noexcept
    {
        const float current = target.load(std::memory_order_relaxed);
        const float updated = (newBlockValue > current) ? newBlockValue : (current * 0.85f + newBlockValue * 0.15f);
        target.store(updated, std::memory_order_relaxed);
    }

    double sampleRate { 44100.0 };

    int maxDelaySamples { 2048 };
    int latencySamples { 0 };
    int delayWriteIndex { 0 };

    std::vector<float> dryDelayBufferL;
    std::vector<float> dryDelayBufferR;

    std::atomic<float> peakLeft  { 0.0f };
    std::atomic<float> peakRight { 0.0f };
    std::atomic<float> rmsLeft   { 0.0f };
    std::atomic<float> rmsRight  { 0.0f };
};

} // namespace camelphat::dsp
