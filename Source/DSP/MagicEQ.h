#pragma once

#include "IDSPModule.h"
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace camelphat::dsp {

/**
 * @brief Module 5: Magic EQ (Resonant Low-End Enhancer)
 *
 * - Resonant 2-pole peaking filter tuned to meq_tune (60 Hz to 120 Hz, default 80 Hz)
 *   where meq_amount controls gain (0 dB to +12 dB).
 * - Complimentary dip at 2 * meq_tune (0 dB to -2.5 dB) to prevent mud.
 * - Phat Mode (meq_phat = 1):
 *   Soft clipper (tanh) directly inside the filter's resonant state feedback path,
 *   plus a gentle +3 dB high-shelf air boost at 8 kHz scaled by meq_amount.
 */
class MagicEQ : public IAudioProcessorModule
{
public:
    MagicEQ() = default;
    ~MagicEQ() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;

        highShelfL.prepare(spec);
        highShelfR.prepare(spec);

        reset();
    }

    void reset() noexcept override
    {
        boostS1L = 0.0f; boostS2L = 0.0f;
        boostS1R = 0.0f; boostS2R = 0.0f;

        dipS1L = 0.0f; dipS2L = 0.0f;
        dipS1R = 0.0f; dipS2R = 0.0f;

        highShelfL.reset();
        highShelfR.reset();
    }

    void setParameters(float amountNorm, float tuneHz, bool phatModeEnabled, bool bypass = false) noexcept
    {
        amount = juce::jlimit(0.0f, 1.0f, amountNorm);
        tune = juce::jlimit(60.0f, 120.0f, tuneHz);
        phatMode = phatModeEnabled;
        isBypassed = bypass;

        updateCoefficients();
    }

    void process(juce::AudioBuffer<float>& buffer) noexcept override
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(2, buffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0 || isBypassed || amount <= 0.001f)
            return;

        float* channelLeft  = buffer.getWritePointer(0);
        float* channelRight = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            // --- Left Channel ---
            const float inL = channelLeft[i];

            // 1. Boost peaking filter at tune Hz
            float boostOutL = b0_boost * inL + boostS1L;
            if (phatMode)
            {
                // Soft clipper tanh inside state feedback path
                boostS1L = b1_boost * inL - a1_boost * std::tanh(boostOutL * 1.2f) + boostS2L;
                boostS2L = b2_boost * inL - a2_boost * std::tanh(boostOutL * 1.2f);
            }
            else
            {
                boostS1L = b1_boost * inL - a1_boost * boostOutL + boostS2L;
                boostS2L = b2_boost * inL - a2_boost * boostOutL;
            }

            // 2. Complimentary anti-mud dip filter at 2 * tune Hz
            float outL = b0_dip * boostOutL + dipS1L;
            dipS1L = b1_dip * boostOutL - a1_dip * outL + dipS2L;
            dipS2L = b2_dip * boostOutL - a2_dip * outL;

            channelLeft[i] = outL;

            // --- Right Channel ---
            if (channelRight != nullptr)
            {
                const float inR = channelRight[i];

                float boostOutR = b0_boost * inR + boostS1R;
                if (phatMode)
                {
                    boostS1R = b1_boost * inR - a1_boost * std::tanh(boostOutR * 1.2f) + boostS2R;
                    boostS2R = b2_boost * inR - a2_boost * std::tanh(boostOutR * 1.2f);
                }
                else
                {
                    boostS1R = b1_boost * inR - a1_boost * boostOutR + boostS2R;
                    boostS2R = b2_boost * inR - a2_boost * boostOutR;
                }

                float outR = b0_dip * boostOutR + dipS1R;
                dipS1R = b1_dip * boostOutR - a1_dip * outR + dipS2R;
                dipS2R = b2_dip * boostOutR - a2_dip * outR;

                channelRight[i] = outR;
            }
        }

        // 3. Phat Mode: gentle +3 dB high-shelf air boost at 8 kHz scaled by amount
        if (phatMode && amount > 0.01f)
        {
            auto shelfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRate, 8000.0f, 0.7071f, juce::Decibels::decibelsToGain(3.0f * amount));
            highShelfL.coefficients = shelfCoeffs;
            highShelfR.coefficients = shelfCoeffs;

            for (int i = 0; i < numSamples; ++i)
            {
                channelLeft[i] = highShelfL.processSample(channelLeft[i]);
                if (channelRight != nullptr)
                    channelRight[i] = highShelfR.processSample(channelRight[i]);
            }
        }
    }

private:
    void updateCoefficients() noexcept
    {
        if (sampleRate <= 0.0)
            return;

        const float fs = static_cast<float>(sampleRate);

        // 1. Peaking Boost at tune Hz (0 dB to +12 dB)
        {
            const float boostDb = amount * 12.0f;
            const float A = std::pow(10.0f, boostDb / 40.0f);
            const float w0 = 2.0f * juce::MathConstants<float>::pi * tune / fs;
            const float cosW0 = std::cos(w0);
            const float sinW0 = std::sin(w0);
            constexpr float q = 1.6f;
            const float alpha = sinW0 / (2.0f * q);

            const float a0 = 1.0f + alpha / A;
            b0_boost = (1.0f + alpha * A) / a0;
            b1_boost = (-2.0f * cosW0) / a0;
            b2_boost = (1.0f - alpha * A) / a0;
            a1_boost = (-2.0f * cosW0) / a0;
            a2_boost = (1.0f - alpha / A) / a0;
        }

        // 2. Complimentary Dip at 2 * tune Hz (0 dB to -2.5 dB)
        {
            const float dipFreq = juce::jmin(fs * 0.48f, 2.0f * tune);
            const float dipDb = -2.5f * amount;
            const float A = std::pow(10.0f, dipDb / 40.0f);
            const float w0 = 2.0f * juce::MathConstants<float>::pi * dipFreq / fs;
            const float cosW0 = std::cos(w0);
            const float sinW0 = std::sin(w0);
            constexpr float q = 2.0f;
            const float alpha = sinW0 / (2.0f * q);

            const float a0 = 1.0f + alpha / A;
            b0_dip = (1.0f + alpha * A) / a0;
            b1_dip = (-2.0f * cosW0) / a0;
            b2_dip = (1.0f - alpha * A) / a0;
            a1_dip = (-2.0f * cosW0) / a0;
            a2_dip = (1.0f - alpha / A) / a0;
        }
    }

    double sampleRate { 44100.0 };
    float amount { 0.0f };
    float tune { 80.0f };
    bool phatMode { false };
    bool isBypassed { false };

    // Biquad coefficients: Boost filter
    float b0_boost { 1.0f }, b1_boost { 0.0f }, b2_boost { 0.0f };
    float a1_boost { 0.0f }, a2_boost { 0.0f };
    float boostS1L { 0.0f }, boostS2L { 0.0f };
    float boostS1R { 0.0f }, boostS2R { 0.0f };

    // Biquad coefficients: Anti-mud Dip filter
    float b0_dip { 1.0f }, b1_dip { 0.0f }, b2_dip { 0.0f };
    float a1_dip { 0.0f }, a2_dip { 0.0f };
    float dipS1L { 0.0f }, dipS2L { 0.0f };
    float dipS1R { 0.0f }, dipS2R { 0.0f };

    // High Shelf for Phat Mode air
    juce::dsp::IIR::Filter<float> highShelfL;
    juce::dsp::IIR::Filter<float> highShelfR;
};

} // namespace camelphat::dsp
