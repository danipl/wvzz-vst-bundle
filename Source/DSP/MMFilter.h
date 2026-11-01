#pragma once

#include "IDSPModule.h"
#include "EnvelopeFollower.h"
#include "BPFilter.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <cmath>
#include <array>

namespace camelphat::dsp {

/**
 * @brief Module 3: MM Filter (12 Multi-Mode Topologies + Envelope Follower)
 *
 * 0: LP 12dB (2-pole ZDF SVF Low-Pass)
 * 1: LP 24dB (4-pole cascaded ZDF SVF with internal tanh feedback saturation)
 * 2: BP 12dB (2-pole Band-Pass)
 * 3: BP 24dB (4-pole Band-Pass)
 * 4: HP 12dB (2-pole High-Pass)
 * 5: HP 24dB (4-pole High-Pass)
 * 6: Notch (2-pole Band-Reject)
 * 7: Peak (Resonant Peaking EQ boost)
 * 8: Comb + (Feedforward/Feedback delay line, positive feedback)
 * 9: Comb - (Feedforward/Feedback delay line, negative feedback)
 * 10: RingMod (Multiplies signal by sine oscillator, resonance controls feedback)
 * 11: Vowel / Formant (3 parallel bandpass filters morphing A-E-I-O-U via mm_cutoff)
 */
class MMFilter : public IDSPModule
{
public:
    MMFilter() = default;
    ~MMFilter() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;

        envFollower.prepare(sampleRate);

        // Pre-allocate comb delay buffer (20 Hz minimum cutoff -> max delay 50 ms + margin)
        maxCombDelaySamples = static_cast<int>(sampleRate * 0.06) + 16;
        combBufferL.assign(static_cast<size_t>(maxCombDelaySamples), 0.0f);
        combBufferR.assign(static_cast<size_t>(maxCombDelaySamples), 0.0f);

        reset();
    }

    void reset() noexcept override
    {
        envFollower.reset();

        svf1L.reset();
        svf1R.reset();
        svf2L.reset();
        svf2R.reset();

        vowelSvf1L.reset(); vowelSvf1R.reset();
        vowelSvf2L.reset(); vowelSvf2R.reset();
        vowelSvf3L.reset(); vowelSvf3R.reset();

        std::fill(combBufferL.begin(), combBufferL.end(), 0.0f);
        std::fill(combBufferR.begin(), combBufferR.end(), 0.0f);
        combWriteIndex = 0;

        ringPhase = 0.0;
        ringFeedbackSampleL = 0.0f;
        ringFeedbackSampleR = 0.0f;
    }

    void setParameters(float cutoffHz,
                       float resonanceNorm,
                       int typeIndex,
                       float mixNorm,
                       float envAttMs,
                       float envRelMs,
                       float envDepthNorm,
                       bool bypass = false) noexcept
    {
        baseCutoff = juce::jlimit(20.0f, static_cast<float>(sampleRate * 0.49), cutoffHz);
        resonance  = juce::jlimit(0.0f, 1.0f, resonanceNorm);
        filterType = juce::jlimit(0, 11, typeIndex);
        filterMix  = juce::jlimit(0.0f, 1.0f, mixNorm);
        envDepth   = juce::jlimit(-1.0f, 1.0f, envDepthNorm);
        isBypassed = bypass;

        envFollower.setAttackRelease(envAttMs, envRelMs);
    }

    /**
     * @brief Process wet passband audio, taking envelope follower input from pre-BP input.
     */
    void processWithSidechain(juce::AudioBuffer<float>& passbandBuffer,
                              const juce::AudioBuffer<float>& sidechainBuffer) noexcept
    {
        const int numSamples = passbandBuffer.getNumSamples();
        const int numChannels = juce::jmin(2, passbandBuffer.getNumChannels(), sidechainBuffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0 || isBypassed)
            return;

        const float maxFreq = static_cast<float>(sampleRate * 0.49);

        // Q mapping: linear [0.707, 12.0]
        const float q = juce::jmap(resonance, 0.707f, 12.0f);
        const float k = 1.0f / q;

        // Formant vowels frequencies table (F1, F2, F3 in Hz) for [A, E, I, O, U]
        static constexpr std::array<float, 5> formantF1 = { 800.0f,  400.0f,  300.0f,  500.0f,  350.0f };
        static constexpr std::array<float, 5> formantF2 = { 1200.0f, 1800.0f, 2200.0f, 900.0f,  750.0f };
        static constexpr std::array<float, 5> formantF3 = { 2500.0f, 2700.0f, 3000.0f, 2600.0f, 2400.0f };

        float* passL = passbandBuffer.getWritePointer(0);
        float* passR = (numChannels > 1) ? passbandBuffer.getWritePointer(1) : nullptr;

        const float* scL = sidechainBuffer.getReadPointer(0);
        const float* scR = (numChannels > 1) ? sidechainBuffer.getReadPointer(1) : scL;

        for (int i = 0; i < numSamples; ++i)
        {
            const float inL = passL[i];
            const float inR = (passR != nullptr) ? passR[i] : inL;

            // 1. Envelope Follower on Pre-BP Input (sidechainBuffer)
            const float scMono = 0.5f * (scL[i] + scR[i]);
            const float env = envFollower.processSample(scMono);

            // 2. Modulated Cutoff (+/- 6 octaves)
            float modCutoff = baseCutoff;
            if (std::abs(envDepth) > 0.001f)
            {
                const float octShift = envDepth * env * 6.0f;
                modCutoff *= std::pow(2.0f, octShift);
                modCutoff = juce::jlimit(20.0f, maxFreq, modCutoff);
            }

            const float g = std::tan(juce::MathConstants<float>::pi * modCutoff / static_cast<float>(sampleRate));

            float wetL = 0.0f;
            float wetR = 0.0f;

            switch (filterType)
            {
                case 0: // LP 12dB
                {
                    wetL = svf1L.processSample(inL, g, k).lp;
                    wetR = svf1R.processSample(inR, g, k).lp;
                    break;
                }
                case 1: // LP 24dB (4-pole cascaded with internal tanh feedback saturation)
                {
                    // Stage 1
                    auto s1OutL = svf1L.processSample(inL, g, k);
                    auto s1OutR = svf1R.processSample(inR, g, k);
                    // Stage 2 with soft feedback saturation
                    const float st1L = std::tanh(s1OutL.lp);
                    const float st1R = std::tanh(s1OutR.lp);
                    wetL = svf2L.processSample(st1L, g, k).lp;
                    wetR = svf2R.processSample(st1R, g, k).lp;
                    break;
                }
                case 2: // BP 12dB
                {
                    wetL = svf1L.processSample(inL, g, k).bp;
                    wetR = svf1R.processSample(inR, g, k).bp;
                    break;
                }
                case 3: // BP 24dB
                {
                    auto s1OutL = svf1L.processSample(inL, g, k);
                    auto s1OutR = svf1R.processSample(inR, g, k);
                    wetL = svf2L.processSample(s1OutL.bp, g, k).bp;
                    wetR = svf2R.processSample(s1OutR.bp, g, k).bp;
                    break;
                }
                case 4: // HP 12dB
                {
                    wetL = svf1L.processSample(inL, g, k).hp;
                    wetR = svf1R.processSample(inR, g, k).hp;
                    break;
                }
                case 5: // HP 24dB
                {
                    auto s1OutL = svf1L.processSample(inL, g, k);
                    auto s1OutR = svf1R.processSample(inR, g, k);
                    wetL = svf2L.processSample(s1OutL.hp, g, k).hp;
                    wetR = svf2R.processSample(s1OutR.hp, g, k).hp;
                    break;
                }
                case 6: // Notch
                {
                    auto outL = svf1L.processSample(inL, g, k);
                    auto outR = svf1R.processSample(inR, g, k);
                    wetL = outL.lp + outL.hp;
                    wetR = outR.lp + outR.hp;
                    break;
                }
                case 7: // Peak
                {
                    auto outL = svf1L.processSample(inL, g, k);
                    auto outR = svf1R.processSample(inR, g, k);
                    // Boost peaking band
                    wetL = inL + outL.bp * (q * 1.2f);
                    wetR = inR + outR.bp * (q * 1.2f);
                    break;
                }
                case 8: // Comb +
                case 9: // Comb -
                {
                    const float delayTimeSec = 1.0f / modCutoff;
                    const float delaySamples = juce::jlimit(1.0f, static_cast<float>(maxCombDelaySamples - 4), delayTimeSec * static_cast<float>(sampleRate));
                    const float sign = (filterType == 8) ? 1.0f : -1.0f;
                    const float feedback = sign * (0.15f + 0.82f * resonance);

                    const float delayedL = readLagrange(combBufferL, combWriteIndex, delaySamples);
                    const float delayedR = readLagrange(combBufferR, combWriteIndex, delaySamples);

                    const float writeL = inL + std::tanh(feedback * delayedL);
                    const float writeR = inR + std::tanh(feedback * delayedR);

                    combBufferL[static_cast<size_t>(combWriteIndex)] = writeL;
                    combBufferR[static_cast<size_t>(combWriteIndex)] = writeR;

                    wetL = 0.5f * (inL + delayedL);
                    wetR = 0.5f * (inR + delayedR);

                    if (++combWriteIndex >= maxCombDelaySamples)
                        combWriteIndex = 0;
                    break;
                }
                case 10: // RingMod
                {
                    const double phaseInc = (juce::MathConstants<double>::twoPi * modCutoff) / sampleRate;
                    const float carrier = static_cast<float>(std::sin(ringPhase));
                    ringPhase += phaseInc;
                    if (ringPhase >= juce::MathConstants<double>::twoPi)
                        ringPhase -= juce::MathConstants<double>::twoPi;

                    // Resonance controls feedback into carrier modulation
                    const float fbL = std::tanh(resonance * 0.8f * ringFeedbackSampleL);
                    const float fbR = std::tanh(resonance * 0.8f * ringFeedbackSampleR);

                    wetL = inL * (carrier + fbL);
                    wetR = inR * (carrier + fbR);

                    ringFeedbackSampleL = wetL;
                    ringFeedbackSampleR = wetR;
                    break;
                }
                case 11: // Vowel / Formant
                {
                    // Morph A-E-I-O-U across normalized cutoff [0, 4]
                    const float normCut = juce::jlimit(0.0f, 4.0f, std::log2(modCutoff / 20.0f) * (4.0f / 9.965f));
                    const int idx = juce::jlimit(0, 3, static_cast<int>(normCut));
                    const float frac = normCut - static_cast<float>(idx);

                    const float f1 = (1.0f - frac) * formantF1[static_cast<size_t>(idx)] + frac * formantF1[static_cast<size_t>(idx + 1)];
                    const float f2 = (1.0f - frac) * formantF2[static_cast<size_t>(idx)] + frac * formantF2[static_cast<size_t>(idx + 1)];
                    const float f3 = (1.0f - frac) * formantF3[static_cast<size_t>(idx)] + frac * formantF3[static_cast<size_t>(idx + 1)];

                    const float qVowel = 4.0f + resonance * 14.0f;
                    const float kVowel = 1.0f / qVowel;

                    const float g1 = std::tan(juce::MathConstants<float>::pi * f1 / static_cast<float>(sampleRate));
                    const float g2 = std::tan(juce::MathConstants<float>::pi * f2 / static_cast<float>(sampleRate));
                    const float g3 = std::tan(juce::MathConstants<float>::pi * f3 / static_cast<float>(sampleRate));

                    const float vL1 = vowelSvf1L.processSample(inL, g1, kVowel).bp;
                    const float vL2 = vowelSvf2L.processSample(inL, g2, kVowel).bp;
                    const float vL3 = vowelSvf3L.processSample(inL, g3, kVowel).bp;

                    const float vR1 = vowelSvf1R.processSample(inR, g1, kVowel).bp;
                    const float vR2 = vowelSvf2R.processSample(inR, g2, kVowel).bp;
                    const float vR3 = vowelSvf3R.processSample(inR, g3, kVowel).bp;

                    wetL = (vL1 + vL2 * 0.7f + vL3 * 0.4f) * 1.5f;
                    wetR = (vR1 + vR2 * 0.7f + vR3 * 0.4f) * 1.5f;
                    break;
                }
            }

            // Wet/Dry mix
            passL[i] = (1.0f - filterMix) * inL + filterMix * wetL;
            if (passR != nullptr)
                passR[i] = (1.0f - filterMix) * inR + filterMix * wetR;
        }
    }

private:
    [[nodiscard]] inline float readLagrange(const std::vector<float>& buf, int currentWrite, float delaySamples) const noexcept
    {
        float readPos = static_cast<float>(currentWrite) - delaySamples;
        while (readPos < 0.0f)
            readPos += static_cast<float>(maxCombDelaySamples);

        const int i1 = static_cast<int>(readPos);
        const int i0 = (i1 - 1 + maxCombDelaySamples) % maxCombDelaySamples;
        const int i2 = (i1 + 1) % maxCombDelaySamples;
        const int i3 = (i1 + 2) % maxCombDelaySamples;

        const float d = readPos - static_cast<float>(i1);

        const float y0 = buf[static_cast<size_t>(i0)];
        const float y1 = buf[static_cast<size_t>(i1)];
        const float y2 = buf[static_cast<size_t>(i2)];
        const float y3 = buf[static_cast<size_t>(i3)];

        // 3rd-order Lagrange interpolation
        const float c0 = -d * (d - 1.0f) * (d - 2.0f) / 6.0f;
        const float c1 = (d + 1.0f) * (d - 1.0f) * (d - 2.0f) / 2.0f;
        const float c2 = -(d + 1.0f) * d * (d - 2.0f) / 2.0f;
        const float c3 = (d + 1.0f) * d * (d - 1.0f) / 6.0f;

        return c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3;
    }

    double sampleRate { 44100.0 };

    float baseCutoff { 1000.0f };
    float resonance  { 0.2f };
    int   filterType { 1 };
    float filterMix  { 1.0f };
    float envDepth   { 0.0f };
    bool  isBypassed { false };

    EnvelopeFollower envFollower;

    // Dual SVF for 12/24dB topologies
    CytomicSVF svf1L, svf1R;
    CytomicSVF svf2L, svf2R;

    // 3 parallel SVF for vowel formant filters
    CytomicSVF vowelSvf1L, vowelSvf1R;
    CytomicSVF vowelSvf2L, vowelSvf2R;
    CytomicSVF vowelSvf3L, vowelSvf3R;

    // Comb filter state
    int maxCombDelaySamples { 0 };
    int combWriteIndex { 0 };
    std::vector<float> combBufferL;
    std::vector<float> combBufferR;

    // RingMod state
    double ringPhase { 0.0 };
    float ringFeedbackSampleL { 0.0f };
    float ringFeedbackSampleR { 0.0f };
};

} // namespace camelphat::dsp
