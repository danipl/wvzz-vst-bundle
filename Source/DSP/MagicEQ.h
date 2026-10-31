#pragma once

#include "IDSPModule.h"
#include <juce_core/juce_core.h>
#include <cmath>

namespace camelphat::dsp {

/**
 * @brief Resonant sub-bass EQ boost with non-linear saturation.
 * Uses a zero-allocation Direct Form II Transposed peaking biquad filter.
 * Faithfully recreates CamelPhat 3's famous "Magic EQ" low-end thickener with strictly 0 heap allocations.
 */
class MagicEQ : public IAudioProcessorModule
{
public:
    MagicEQ() = default;
    ~MagicEQ() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        reset();
    }

    void reset() noexcept override
    {
        s1L = 0.0f;
        s2L = 0.0f;
        s1R = 0.0f;
        s2R = 0.0f;
    }

    void setParameters(float amountNorm, float frequencyHz) noexcept
    {
        amount = juce::jlimit(0.0f, 1.0f, amountNorm);
        frequency = juce::jlimit(30.0f, 120.0f, frequencyHz);
        updateCoefficients();
    }

    void process(juce::AudioBuffer<float>& buffer) noexcept override
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(2, buffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0 || amount <= 0.001f)
            return;

        float* channelLeft  = buffer.getWritePointer(0);
        float* channelRight = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            // Process Left Channel through Transposed Direct Form II
            const float inL = channelLeft[i];
            float boostedL = b0 * inL + s1L;
            s1L = b1 * inL - a1 * boostedL + s2L;
            s2L = b2 * inL - a2 * boostedL;

            // Warm sub-bass saturation
            if (amount > 0.05f)
            {
                const float drive = 1.0f + amount * 1.5f;
                boostedL = std::tanh(boostedL * drive) / drive;
            }
            channelLeft[i] = boostedL;

            // Process Right Channel
            if (channelRight != nullptr)
            {
                const float inR = channelRight[i];
                float boostedR = b0 * inR + s1R;
                s1R = b1 * inR - a1 * boostedR + s2R;
                s2R = b2 * inR - a2 * boostedR;

                if (amount > 0.05f)
                {
                    const float drive = 1.0f + amount * 1.5f;
                    boostedR = std::tanh(boostedR * drive) / drive;
                }
                channelRight[i] = boostedR;
            }
        }
    }

private:
    void updateCoefficients() noexcept
    {
        if (sampleRate <= 0.0)
            return;

        // Audio EQ Cookbook Peaking Filter (calculated on stack, zero heap allocation)
        const float boostDb = amount * 12.0f;
        const float A = std::pow(10.0f, boostDb / 40.0f);
        const float w0 = 2.0f * juce::MathConstants<float>::pi * frequency / static_cast<float>(sampleRate);
        const float cosW0 = std::cos(w0);
        const float sinW0 = std::sin(w0);
        const float q = 1.8f; // Musical resonance
        const float alpha = sinW0 / (2.0f * q);

        const float a0 = 1.0f + alpha / A;
        b0 = (1.0f + alpha * A) / a0;
        b1 = (-2.0f * cosW0) / a0;
        b2 = (1.0f - alpha * A) / a0;
        a1 = (-2.0f * cosW0) / a0;
        a2 = (1.0f - alpha / A) / a0;
    }

    double sampleRate { 44100.0 };
    float amount { 0.0f };
    float frequency { 60.0f };

    // Normalized biquad filter coefficients
    float b0 { 1.0f }, b1 { 0.0f }, b2 { 0.0f };
    float a1 { 0.0f }, a2 { 0.0f };

    // Transposed Direct Form II state registers
    float s1L { 0.0f }, s2L { 0.0f };
    float s1R { 0.0f }, s2R { 0.0f };
};

} // namespace camelphat::dsp
