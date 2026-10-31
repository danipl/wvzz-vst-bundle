#pragma once

#include "IDSPModule.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <cmath>

namespace camelphat::dsp {

/**
 * @brief Master mixing stage, output volume control, atomic VU metering, and NaN/Inf safety clamp.
 * Adheres to IMeterSource and IAudioProcessorModule interfaces.
 */
class MasterSection : public IDSPModule, public IMeterSource
{
public:
    MasterSection() = default;
    ~MasterSection() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        reset();
    }

    void reset() noexcept override
    {
        peakLeft.store(0.0f, std::memory_order_relaxed);
        peakRight.store(0.0f, std::memory_order_relaxed);
        rmsLeft.store(0.0f, std::memory_order_relaxed);
        rmsRight.store(0.0f, std::memory_order_relaxed);
    }

    /**
     * @brief Blends wet chain with global dry buffer, applies master gain, clamps NaNs, and computes metering.
     */
    void process(juce::AudioBuffer<float>& wetBuffer,
                 const juce::AudioBuffer<float>& dryBuffer,
                 float mixNorm,
                 float outputGainDb) noexcept
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

        float* wetR = numChannels > 1 ? wetBuffer.getWritePointer(1) : nullptr;
        const float* dryR = numChannels > 1 ? dryBuffer.getReadPointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            // Channel 0 (Left)
            float sampleL = (1.0f - mix) * dryL[i] + mix * wetL[i];
            sampleL *= linearGain;

            // Master NaN / Inf sanitization and speaker protection clamp
            if (!std::isfinite(sampleL))
                sampleL = 0.0f;
            else
                sampleL = juce::jlimit(-8.0f, 8.0f, sampleL);

            wetL[i] = sampleL;
            const float absL = std::abs(sampleL);
            blockPeakL = juce::jmax(blockPeakL, absL);
            blockSumSqL += sampleL * sampleL;

            // Channel 1 (Right)
            if (wetR != nullptr && dryR != nullptr)
            {
                float sampleR = (1.0f - mix) * dryR[i] + mix * wetR[i];
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

        // Update atomic metering values with smooth decay
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
        // Instant attack, fast decay
        const float updated = (newBlockValue > current) ? newBlockValue : (current * 0.85f + newBlockValue * 0.15f);
        target.store(updated, std::memory_order_relaxed);
    }

    double sampleRate { 44100.0 };

    std::atomic<float> peakLeft  { 0.0f };
    std::atomic<float> peakRight { 0.0f };
    std::atomic<float> rmsLeft   { 0.0f };
    std::atomic<float> rmsRight  { 0.0f };
};

} // namespace camelphat::dsp
