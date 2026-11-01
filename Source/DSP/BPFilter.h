#pragma once

#include "IDSPModule.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace camelphat::dsp {

/**
 * @brief Andy Simper (Cytomic) ZDF State Variable Filter (trapezoidal integration).
 * Computes simultaneous LP, BP, HP outputs unconditionally stable up to Nyquist.
 */
class CytomicSVF
{
public:
    CytomicSVF() = default;

    void reset() noexcept
    {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    struct Output
    {
        float lp { 0.0f };
        float bp { 0.0f };
        float hp { 0.0f };
    };

    /**
     * @brief Process a single sample through the ZDF SVF.
     * @param in Input sample.
     * @param g Pre-warped frequency parameter: tan(pi * fc / fs).
     * @param k Damping parameter: 1.0f / Q.
     */
    [[nodiscard]] inline Output processSample(float in, float g, float k) noexcept
    {
        // Andy Simper ZDF SVF equations:
        // v1 = (s1 + g * (in - s2)) / (1 + g * (g + k))
        // v2 = s2 + g * v1
        // lp = v2
        // bp = v1
        // hp = in - k * v1 - v2
        const float denom = 1.0f / (1.0f + g * (g + k));
        const float v1 = (s1 + g * (in - s2)) * denom;
        const float v2 = s2 + g * v1;

        s1 = 2.0f * v1 - s1;
        s2 = 2.0f * v2 - s2;

        Output out;
        out.lp = v2;
        out.bp = v1;
        out.hp = in - k * v1 - v2;
        return out;
    }

private:
    float s1 { 0.0f };
    float s2 { 0.0f };
};

/**
 * @brief Module 1: BP Filter (Bandpass + Band-Reject Split)
 * Uses Andy Simper's Cytomic ZDF SVF topology.
 *
 * Low_Rejected  = LowPass_Output(Input, Low_Cut_Freq, Low_Res)
 * High_Rejected = HighPass_Output(Input, High_Cut_Freq, High_Res)
 * Passband      = BandPass_Cascade(Input)
 */
class BPFilter : public IDSPModule
{
public:
    BPFilter() = default;
    ~BPFilter() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        reset();
    }

    void reset() noexcept override
    {
        svfLowL.reset();
        svfLowR.reset();
        svfHighL.reset();
        svfHighR.reset();
        svfHighCutInL.reset();
        svfHighCutInR.reset();
    }

    /**
     * @brief Splits input stereo audio into passband and band-reject buffers.
     * @param inputBuffer Read-only source audio.
     * @param passbandBuffer Destination for bandpass filtered audio.
     * @param bandRejectBuffer Destination for out-of-band rejected audio (Low_Rejected + High_Rejected).
     * @param cutoffLow High-pass cutoff frequency in Hz (20 - 20000).
     * @param cutoffHigh Low-pass cutoff frequency in Hz (20 - 20000).
     * @param lowResNorm Low resonance (0.0 to 1.0 -> Q: 0.707 to 12.0).
     * @param highResNorm High resonance (0.0 to 1.0 -> Q: 0.707 to 12.0).
     * @param bypass BP Bypass toggle (true: passband = input, bandReject = 0).
     */
    void process(const juce::AudioBuffer<float>& inputBuffer,
                 juce::AudioBuffer<float>& passbandBuffer,
                 juce::AudioBuffer<float>& bandRejectBuffer,
                 float cutoffLow,
                 float cutoffHigh,
                 float lowResNorm,
                 float highResNorm,
                 bool bypass = false) noexcept
    {
        const int numSamples = inputBuffer.getNumSamples();
        const int numChannels = juce::jmin(2, inputBuffer.getNumChannels(), passbandBuffer.getNumChannels(), bandRejectBuffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0)
            return;

        if (bypass)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                passbandBuffer.copyFrom(ch, 0, inputBuffer.getReadPointer(ch), numSamples);
                bandRejectBuffer.clear(ch, 0, numSamples);
            }
            return;
        }

        // Q mapping: linear [0.707, 12.0]
        const float qLow = juce::jmap(juce::jlimit(0.0f, 1.0f, lowResNorm), 0.707f, 12.0f);
        const float qHigh = juce::jmap(juce::jlimit(0.0f, 1.0f, highResNorm), 0.707f, 12.0f);

        const float kLow = 1.0f / qLow;
        const float kHigh = 1.0f / qHigh;

        // Clamp frequencies safely within Nyquist limit and enforce Low Cut <= High Cut
        const float maxFreq = static_cast<float>(sampleRate * 0.49);
        const float safeLow  = juce::jlimit(20.0f, maxFreq, cutoffLow);
        const float safeHigh = juce::jlimit(safeLow, maxFreq, cutoffHigh);

        const float gLow = std::tan(juce::MathConstants<float>::pi * safeLow / static_cast<float>(sampleRate));
        const float gHigh = std::tan(juce::MathConstants<float>::pi * safeHigh / static_cast<float>(sampleRate));

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* in = inputBuffer.getReadPointer(ch);
            float* pass = passbandBuffer.getWritePointer(ch);
            float* rej = bandRejectBuffer.getWritePointer(ch);

            auto& svfL = (ch == 0) ? svfLowL : svfLowR;
            auto& svfH = (ch == 0) ? svfHighL : svfHighR;
            auto& svfHCIn = (ch == 0) ? svfHighCutInL : svfHighCutInR;

            for (int i = 0; i < numSamples; ++i)
            {
                const float x = in[i];

                // 1. Low Cut filter on Input: extracts Low_Rejected = LP, and passband intermediate = HP
                auto outLowCut = svfL.processSample(x, gLow, kLow);
                const float lowRejected = outLowCut.lp;
                const float highPassed = outLowCut.hp;

                // 2. High Cut filter on highPassed signal: extracts Passband = LP(highPassed)
                auto outHighCut = svfH.processSample(highPassed, gHigh, kHigh);
                const float passband = outHighCut.lp;

                // 3. High Cut filter on Input directly to extract High_Rejected = HP(Input)
                auto outHighRejected = svfHCIn.processSample(x, gHigh, kHigh);
                const float highRejected = outHighRejected.hp;

                pass[i] = passband;
                rej[i] = lowRejected + highRejected;
            }
        }
    }

private:
    double sampleRate { 44100.0 };

    CytomicSVF svfLowL;
    CytomicSVF svfLowR;
    CytomicSVF svfHighL;
    CytomicSVF svfHighR;
    CytomicSVF svfHighCutInL;
    CytomicSVF svfHighCutInR;
};

} // namespace camelphat::dsp
