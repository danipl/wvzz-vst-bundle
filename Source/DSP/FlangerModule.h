#pragma once

#include "IDSPModule.h"
#include <vector>
#include <cmath>

namespace camelphat::dsp {

/**
 * @brief Stereo modulated flanger with quadrature LFO and soft-saturating feedback.
 * Provides rich stereo spatialization and classic analog comb-filtering sweeps.
 */
class FlangerModule : public IAudioProcessorModule
{
public:
    FlangerModule() = default;
    ~FlangerModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;

        // Allocate circular delay buffer (maximum 20ms at max sample rate 192kHz)
        maxDelaySamples = static_cast<int>(sampleRate * 0.025) + 8;
        delayBufferL.assign(static_cast<size_t>(maxDelaySamples), 0.0f);
        delayBufferR.assign(static_cast<size_t>(maxDelaySamples), 0.0f);

        reset();
    }

    void reset() noexcept override
    {
        std::fill(delayBufferL.begin(), delayBufferL.end(), 0.0f);
        std::fill(delayBufferR.begin(), delayBufferR.end(), 0.0f);

        writeIndex = 0;
        lfoPhase = 0.0;
        feedbackSampleL = 0.0f;
        feedbackSampleR = 0.0f;
    }

    void setParameters(float rateHz, float depthNorm, float feedbackNorm, float mixNorm) noexcept
    {
        rate = juce::jlimit(0.01f, 10.0f, rateHz);
        depth = juce::jlimit(0.0f, 1.0f, depthNorm);
        feedback = juce::jlimit(-0.95f, 0.95f, feedbackNorm);
        flangerMix = juce::jlimit(0.0f, 1.0f, mixNorm);
    }

    void process(juce::AudioBuffer<float>& buffer) noexcept override
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(2, buffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0 || flangerMix <= 0.001f || maxDelaySamples <= 0)
            return;

        float* channelLeft  = buffer.getWritePointer(0);
        float* channelRight = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

        const double phaseIncrement = (juce::MathConstants<double>::twoPi * rate) / sampleRate;
        const float baseDelaySamples = static_cast<float>(sampleRate * 0.002); // 2ms base
        const float maxModSamples = static_cast<float>(sampleRate * 0.005) * depth; // up to 5ms sweep

        for (int i = 0; i < numSamples; ++i)
        {
            // LFO with 90-degree (pi/2) stereo quadrature phase spread
            const float lfoL = 0.5f * (1.0f + static_cast<float>(std::sin(lfoPhase)));
            const float lfoR = 0.5f * (1.0f + static_cast<float>(std::sin(lfoPhase + juce::MathConstants<double>::halfPi)));

            lfoPhase += phaseIncrement;
            if (lfoPhase >= juce::MathConstants<double>::twoPi)
                lfoPhase -= juce::MathConstants<double>::twoPi;

            // Target delay in samples
            const float delayL = baseDelaySamples + maxModSamples * lfoL;
            const float delayR = baseDelaySamples + maxModSamples * lfoR;

            // Left Channel
            const float inL = channelLeft[i];
            const float delayedL = readInterpolated(delayBufferL, writeIndex, delayL);
            // Soft-saturated feedback prevents runaway oscillation
            const float writeValL = inL + std::tanh(feedback * feedbackSampleL);
            delayBufferL[static_cast<size_t>(writeIndex)] = writeValL;
            feedbackSampleL = delayedL;
            channelLeft[i] = (1.0f - flangerMix) * inL + flangerMix * delayedL;

            // Right Channel
            if (channelRight != nullptr)
            {
                const float inR = channelRight[i];
                const float delayedR = readInterpolated(delayBufferR, writeIndex, delayR);
                const float writeValR = inR + std::tanh(feedback * feedbackSampleR);
                delayBufferR[static_cast<size_t>(writeIndex)] = writeValR;
                feedbackSampleR = delayedR;
                channelRight[i] = (1.0f - flangerMix) * inR + flangerMix * delayedR;
            }

            if (++writeIndex >= maxDelaySamples)
                writeIndex = 0;
        }
    }

private:
    [[nodiscard]] inline float readInterpolated(const std::vector<float>& buf, int currentWrite, float delay) const noexcept
    {
        float readPos = static_cast<float>(currentWrite) - delay;
        while (readPos < 0.0f)
            readPos += static_cast<float>(maxDelaySamples);

        const int i0 = static_cast<int>(readPos);
        const int i1 = (i0 + 1) >= maxDelaySamples ? 0 : i0 + 1;
        const float frac = readPos - static_cast<float>(i0);

        return (1.0f - frac) * buf[static_cast<size_t>(i0)] + frac * buf[static_cast<size_t>(i1)];
    }

    double sampleRate { 44100.0 };
    int maxDelaySamples { 0 };
    int writeIndex { 0 };

    float rate { 0.2f };
    float depth { 0.0f };
    float feedback { 0.0f };
    float flangerMix { 0.0f };

    double lfoPhase { 0.0 };
    float feedbackSampleL { 0.0f };
    float feedbackSampleR { 0.0f };

    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
};

} // namespace camelphat::dsp
