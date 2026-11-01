#pragma once

#include "IDSPModule.h"
#include <vector>
#include <cmath>

namespace camelphat::dsp {

/**
 * @brief Module 4: Flanger (Stereo Modulated)
 *
 * - Delay line using 3rd-order Lagrange fractional interpolation
 * - Center delay time: 2.5 ms
 * - Sweep depth: +/- 2.0 ms
 * - Fixed internal feedback: 0.55
 * - Quadrature LFO phase offset (90 degrees between Left and Right)
 * - flg_rate: 0.01 to 20.0 Hz (default 0.4 Hz)
 * - flg_amount: 0.0 to 1.0 (wet/dry mix)
 * - flg_bypass: default 1 (bypassed)
 */
class FlangerModule : public IAudioProcessorModule
{
public:
    FlangerModule() = default;
    ~FlangerModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;

        // Max delay: 2.5ms + 2.0ms = 4.5ms; allocate 15ms buffer
        maxDelaySamples = static_cast<int>(sampleRate * 0.02) + 16;
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

    void setParameters(float rateHz, float amountNorm, bool bypass = true) noexcept
    {
        rate = juce::jlimit(0.01f, 20.0f, rateHz);
        amount = juce::jlimit(0.0f, 1.0f, amountNorm);
        isBypassed = bypass;
    }

    void process(juce::AudioBuffer<float>& buffer) noexcept override
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(2, buffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0 || isBypassed || amount <= 0.001f || maxDelaySamples <= 0)
            return;

        float* channelLeft  = buffer.getWritePointer(0);
        float* channelRight = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

        const double phaseIncrement = (juce::MathConstants<double>::twoPi * rate) / sampleRate;
        const float centerDelaySamples = static_cast<float>(sampleRate * 0.0025); // 2.5ms
        const float sweepModSamples    = static_cast<float>(sampleRate * 0.0020); // +/- 2.0ms
        constexpr float fixedFeedback  = 0.55f;

        for (int i = 0; i < numSamples; ++i)
        {
            // Quadrature LFO (90 degree phase shift)
            const float lfoL = static_cast<float>(std::sin(lfoPhase));
            const float lfoR = static_cast<float>(std::sin(lfoPhase + juce::MathConstants<double>::halfPi));

            lfoPhase += phaseIncrement;
            if (lfoPhase >= juce::MathConstants<double>::twoPi)
                lfoPhase -= juce::MathConstants<double>::twoPi;

            const float delayL = centerDelaySamples + sweepModSamples * lfoL;
            const float delayR = centerDelaySamples + sweepModSamples * lfoR;

            // Left Channel
            const float inL = channelLeft[i];
            const float delayedL = readLagrange(delayBufferL, writeIndex, delayL);
            const float writeValL = inL + std::tanh(fixedFeedback * feedbackSampleL);
            delayBufferL[static_cast<size_t>(writeIndex)] = writeValL;
            feedbackSampleL = delayedL;
            channelLeft[i] = (1.0f - amount) * inL + amount * delayedL;

            // Right Channel
            if (channelRight != nullptr)
            {
                const float inR = channelRight[i];
                const float delayedR = readLagrange(delayBufferR, writeIndex, delayR);
                const float writeValR = inR + std::tanh(fixedFeedback * feedbackSampleR);
                delayBufferR[static_cast<size_t>(writeIndex)] = writeValR;
                feedbackSampleR = delayedR;
                channelRight[i] = (1.0f - amount) * inR + amount * delayedR;
            }

            if (++writeIndex >= maxDelaySamples)
                writeIndex = 0;
        }
    }

private:
    [[nodiscard]] inline float readLagrange(const std::vector<float>& buf, int currentWrite, float delay) const noexcept
    {
        float readPos = static_cast<float>(currentWrite) - delay;
        while (readPos < 0.0f)
            readPos += static_cast<float>(maxDelaySamples);

        const int i1 = static_cast<int>(readPos);
        const int i0 = (i1 - 1 + maxDelaySamples) % maxDelaySamples;
        const int i2 = (i1 + 1) % maxDelaySamples;
        const int i3 = (i1 + 2) % maxDelaySamples;

        const float d = readPos - static_cast<float>(i1);

        const float y0 = buf[static_cast<size_t>(i0)];
        const float y1 = buf[static_cast<size_t>(i1)];
        const float y2 = buf[static_cast<size_t>(i2)];
        const float y3 = buf[static_cast<size_t>(i3)];

        const float c0 = -d * (d - 1.0f) * (d - 2.0f) / 6.0f;
        const float c1 = (d + 1.0f) * (d - 1.0f) * (d - 2.0f) / 2.0f;
        const float c2 = -(d + 1.0f) * d * (d - 2.0f) / 2.0f;
        const float c3 = (d + 1.0f) * d * (d - 1.0f) / 6.0f;

        return c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3;
    }

    double sampleRate { 44100.0 };
    int maxDelaySamples { 0 };
    int writeIndex { 0 };

    float rate { 0.4f };
    float amount { 0.0f };
    bool isBypassed { true };

    double lfoPhase { 0.0 };
    float feedbackSampleL { 0.0f };
    float feedbackSampleR { 0.0f };

    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
};

} // namespace camelphat::dsp
