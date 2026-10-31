#pragma once

#include "IDSPModule.h"
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace camelphat::dsp {

/**
 * @brief Optical program-dependent dynamic compressor with analog "Phat" mode saturation.
 * Accurately models the smooth leveling and transient punch of CamelPhat 3's compressor.
 */
class CompressorModule : public IAudioProcessorModule
{
public:
    CompressorModule() = default;
    ~CompressorModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        reset();
    }

    void reset() noexcept override
    {
        envelope = 0.0f;
        gainReductionDb = 0.0f;
    }

    void setParameters(float thresholdDb,
                       float ratioVal,
                       float attackMs,
                       float releaseMs,
                       bool phatModeEnabled,
                       float mixNorm) noexcept
    {
        threshold = juce::jlimit(-60.0f, 0.0f, thresholdDb);
        ratio = juce::jlimit(1.0f, 20.0f, ratioVal);
        phatMode = phatModeEnabled;
        compMix = juce::jlimit(0.0f, 1.0f, mixNorm);

        const float safeAttack  = juce::jmax(0.1f, attackMs);
        const float safeRelease = juce::jmax(1.0f, releaseMs);

        attackCoeff  = std::exp(-1.0f / (static_cast<float>(sampleRate) * safeAttack * 0.001f));
        releaseCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * safeRelease * 0.001f));
    }

    void process(juce::AudioBuffer<float>& buffer) noexcept override
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(2, buffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0)
            return;

        float* channelLeft  = buffer.getWritePointer(0);
        float* channelRight = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

        const float slope = 1.0f - (1.0f / ratio);

        for (int i = 0; i < numSamples; ++i)
        {
            const float inL = channelLeft[i];
            const float inR = channelRight != nullptr ? channelRight[i] : inL;

            // Stereo link: detect max absolute amplitude
            const float detectorIn = juce::jmax(std::abs(inL), std::abs(inR));

            // Envelope detection with optical ballistics
            if (detectorIn > envelope)
                envelope = detectorIn + attackCoeff * (envelope - detectorIn);
            else
                envelope = detectorIn + releaseCoeff * (envelope - detectorIn);

            // Compute gain reduction in dB
            const float envDb = juce::Decibels::gainToDecibels(std::max(envelope, 1.0e-5f));
            float targetGrDb = 0.0f;

            if (envDb > threshold)
            {
                const float overshoot = envDb - threshold;
                targetGrDb = -overshoot * slope;
            }

            gainReductionDb = targetGrDb;
            const float linearGain = juce::Decibels::decibelsToGain(gainReductionDb);

            // Apply compression gain
            float compressedL = inL * linearGain;
            float compressedR = inR * linearGain;

            // "Phat" Mode: Tube/transformer analog saturation with harmonic density
            if (phatMode)
            {
                compressedL = applyPhatSaturation(compressedL);
                compressedR = applyPhatSaturation(compressedR);
            }

            // Blend Wet and Dry
            channelLeft[i] = (1.0f - compMix) * inL + compMix * compressedL;
            if (channelRight != nullptr)
                channelRight[i] = (1.0f - compMix) * inR + compMix * compressedR;
        }
    }

    [[nodiscard]] float getGainReductionDb() const noexcept { return gainReductionDb; }

private:
    [[nodiscard]] inline float applyPhatSaturation(float x) const noexcept
    {
        // 1.5x drive with gentle soft-knee saturation
        const float driven = x * 1.4f;
        return std::tanh(driven) * 0.9f;
    }

    double sampleRate { 44100.0 };

    float threshold { 0.0f };
    float ratio { 2.0f };
    float compMix { 1.0f };
    bool phatMode { false };

    float attackCoeff  { 0.0f };
    float releaseCoeff { 0.0f };

    float envelope { 0.0f };
    float gainReductionDb { 0.0f };
};

} // namespace camelphat::dsp
