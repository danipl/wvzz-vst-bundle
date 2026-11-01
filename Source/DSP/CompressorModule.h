#pragma once

#include "IDSPModule.h"
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <algorithm>

namespace camelphat::dsp {

/**
 * @brief Module 6: Compressor (CamelCrusher / Phat Dynamics)
 *
 * - One-knob feed-forward compressor with auto-makeup gain and safety soft-clipper.
 * - Attack:
 *     Normal: 3.5 ms
 *     Phat:   0.8 ms
 * - comp_amount (0.0 to 1.0):
 *     Simultaneously drives Threshold from 0 dB down to -36 dB
 *     and applies Auto-Makeup Gain from +0 dB to +14 dB.
 * - Normal Mode (comp_phat = 0):
 *     Ratio: 3.5:1, Soft Knee: 6 dB
 * - Phat Mode (comp_phat = 1):
 *     Ratio: 12:1, Hard Knee: 1 dB,
 *     plus post-makeup tanh soft-clip ceiling at -0.1 dBFS.
 * - comp_release: 10.0 to 1500.0 ms (default 120.0 ms)
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

    void setParameters(float amountNorm,
                       float releaseMs,
                       bool phatModeEnabled,
                       bool bypass = false) noexcept
    {
        amount = juce::jlimit(0.0f, 1.0f, amountNorm);
        releaseTimeMs = juce::jlimit(10.0f, 1500.0f, releaseMs);
        phatMode = phatModeEnabled;
        isBypassed = bypass;

        // One-knob mappings:
        // Threshold: 0 dB down to -36 dB
        thresholdDb = -36.0f * amount;

        // Auto-Makeup: 0 dB up to +14 dB
        autoMakeupDb = 14.0f * amount;

        // Ballistics and curve parameters based on mode:
        if (phatMode)
        {
            attackMs = 0.8f;
            ratio = 12.0f;
            kneeWidthDb = 1.0f;
        }
        else
        {
            attackMs = 3.5f;
            ratio = 3.5f;
            kneeWidthDb = 6.0f;
        }

        const float safeAttack = juce::jmax(0.05f, attackMs);
        const float safeRelease = juce::jmax(1.0f, releaseTimeMs);

        attackCoeff  = std::exp(-1.0f / (static_cast<float>(sampleRate) * safeAttack * 0.001f));
        releaseCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * safeRelease * 0.001f));
    }

    void process(juce::AudioBuffer<float>& buffer) noexcept override
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(2, buffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0 || isBypassed || amount <= 0.001f)
            return;

        float* channelLeft  = buffer.getWritePointer(0);
        float* channelRight = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

        const float autoMakeupLinear = juce::Decibels::decibelsToGain(autoMakeupDb);
        const float halfKnee = kneeWidthDb * 0.5f;
        const float slope = 1.0f - (1.0f / ratio);

        // Ceiling threshold in linear for -0.1 dBFS
        constexpr float ceilingLinear = 0.988553f; // -0.1 dBFS
        const float invCeiling = 1.0f / ceilingLinear;

        for (int i = 0; i < numSamples; ++i)
        {
            const float inL = channelLeft[i];
            const float inR = (channelRight != nullptr) ? channelRight[i] : inL;

            // Stereo link: peak detector
            const float detectorIn = juce::jmax(std::abs(inL), std::abs(inR));

            // Envelope detection
            if (detectorIn > envelope)
                envelope = detectorIn + attackCoeff * (envelope - detectorIn);
            else
                envelope = detectorIn + releaseCoeff * (envelope - detectorIn);

            const float envDb = juce::Decibels::gainToDecibels(std::max(envelope, 1.0e-5f));

            // Soft-knee feedforward compression curve
            float grDb = 0.0f;
            if (envDb > thresholdDb + halfKnee)
            {
                grDb = -slope * (envDb - thresholdDb);
            }
            else if (envDb > thresholdDb - halfKnee)
            {
                const float diff = envDb - thresholdDb + halfKnee;
                grDb = -slope * (diff * diff) / (2.0f * kneeWidthDb);
            }

            gainReductionDb = grDb;
            const float compGain = juce::Decibels::decibelsToGain(grDb) * autoMakeupLinear;

            float outL = inL * compGain;
            float outR = inR * compGain;

            // Phat mode post-makeup tanh soft-clip ceiling at -0.1 dBFS
            if (phatMode)
            {
                outL = std::tanh(outL * invCeiling) * ceilingLinear;
                outR = std::tanh(outR * invCeiling) * ceilingLinear;
            }

            channelLeft[i] = outL;
            if (channelRight != nullptr)
                channelRight[i] = outR;
        }
    }

    [[nodiscard]] float getGainReductionDb() const noexcept { return gainReductionDb; }

private:
    double sampleRate { 44100.0 };

    float amount { 0.0f };
    float releaseTimeMs { 120.0f };
    bool  phatMode { false };
    bool  isBypassed { false };

    float thresholdDb { 0.0f };
    float autoMakeupDb { 0.0f };
    float attackMs { 3.5f };
    float ratio { 3.5f };
    float kneeWidthDb { 6.0f };

    float attackCoeff { 0.0f };
    float releaseCoeff { 0.0f };
    float envelope { 0.0f };
    float gainReductionDb { 0.0f };
};

} // namespace camelphat::dsp
