#pragma once

#include "IDSPModule.h"
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <algorithm>

namespace camelphat::dsp {

/**
 * @brief Module 2: Distortion (4 Parallel Circuits + 4x Oversampling)
 *
 * 1. Mech:
 *    - Pre-emphasis peaking filter at 1.2 kHz (+6 dB, Q=1.0)
 *    - Transfer function:
 *        y = (x >= 0) ? tanh(drive * x) + 0.15 * sin(drive * x * pi) : clamp(drive * 1.2 * x, -0.9, 0.0)
 *    - Post-de-emphasis peaking filter at 1.2 kHz (-6 dB, Q=1.0)
 *    - drive: exponential 1.0 to 25.0
 *
 * 2. Tube:
 *    - Soft-knee symmetric/asymmetric transfer:
 *        y = tanh(drive * x + 0.1 * x^2) - tanh(0.1 * x^2)
 *    - drive: exponential 1.0 to 15.0
 *    - DC blocker
 *
 * 3. Bitcrusher:
 *    - Runs at BASE sample rate BEFORE upsampling
 *    - dist_bit 0.0 to 1.0: Bit depth 16-bit down to 4-bit, sample-and-hold 1x to 32x
 *
 * 4. Xcita:
 *    - 2-pole Butterworth HPF at 2.8 kHz
 *    - Fast full-wave rectifier + soft clipper:
 *        y_hf = tanh(drive * (0.7 * x_hf + 0.3 * abs(x_hf)))
 *    - 10 Hz HPF DC blocker, summed with dry signal
 */
class DistortionModule : public IAudioProcessorModule
{
public:
    DistortionModule()
        : oversampling(2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false)
    {
    }

    ~DistortionModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        baseSampleRate = spec.sampleRate;
        oversampling.initProcessing(spec.maximumBlockSize);

        const double osSampleRate = baseSampleRate * oversampling.getOversamplingFactor();
        juce::dsp::ProcessSpec osSpec;
        osSpec.sampleRate = osSampleRate;
        osSpec.maximumBlockSize = spec.maximumBlockSize * static_cast<juce::uint32>(oversampling.getOversamplingFactor());
        osSpec.numChannels = spec.numChannels;

        // Mech Pre-emphasis & De-emphasis filters operating at oversampled rate
        mechPreFilterL.prepare(osSpec);
        mechPreFilterR.prepare(osSpec);
        mechPostFilterL.prepare(osSpec);
        mechPostFilterR.prepare(osSpec);

        auto preCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(osSampleRate, 1200.0f, 1.0f, juce::Decibels::decibelsToGain(6.0f));
        auto postCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(osSampleRate, 1200.0f, 1.0f, juce::Decibels::decibelsToGain(-6.0f));

        mechPreFilterL.coefficients = preCoeffs;
        mechPreFilterR.coefficients = preCoeffs;
        mechPostFilterL.coefficients = postCoeffs;
        mechPostFilterR.coefficients = postCoeffs;

        // Tube 10 Hz DC blocker (oversampled)
        tubeDcBlockerL.prepare(osSpec);
        tubeDcBlockerR.prepare(osSpec);
        auto dcCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(osSampleRate, 10.0f);
        tubeDcBlockerL.coefficients = dcCoeffs;
        tubeDcBlockerR.coefficients = dcCoeffs;

        // Xcita 2.8 kHz 2-pole Butterworth HPF + 10 Hz DC blocker (oversampled)
        xcitaHpfL.prepare(osSpec);
        xcitaHpfR.prepare(osSpec);
        auto xcitaCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(osSampleRate, 2800.0f, 0.7071f);
        xcitaHpfL.coefficients = xcitaCoeffs;
        xcitaHpfR.coefficients = xcitaCoeffs;

        xcitaDcBlockerL.prepare(osSpec);
        xcitaDcBlockerR.prepare(osSpec);
        xcitaDcBlockerL.coefficients = dcCoeffs;
        xcitaDcBlockerR.coefficients = dcCoeffs;

        reset();
    }

    void reset() noexcept override
    {
        oversampling.reset();

        mechPreFilterL.reset();
        mechPreFilterR.reset();
        mechPostFilterL.reset();
        mechPostFilterR.reset();

        tubeDcBlockerL.reset();
        tubeDcBlockerR.reset();

        xcitaHpfL.reset();
        xcitaHpfR.reset();
        xcitaDcBlockerL.reset();
        xcitaDcBlockerR.reset();

        bitHoldSampleL = 0.0f;
        bitHoldSampleR = 0.0f;
        bitHoldCounterL = 0;
        bitHoldCounterR = 0;
    }

    void setParameters(float mechNorm, float tubeNorm, float bitNorm, float xcitaNorm, bool bypass = false) noexcept
    {
        mechAmount  = juce::jlimit(0.0f, 1.0f, mechNorm);
        tubeAmount  = juce::jlimit(0.0f, 1.0f, tubeNorm);
        bitAmount   = juce::jlimit(0.0f, 1.0f, bitNorm);
        xcitaAmount = juce::jlimit(0.0f, 1.0f, xcitaNorm);
        isBypassed  = bypass;
    }

    void process(juce::AudioBuffer<float>& buffer) noexcept override
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(2, buffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0 || isBypassed)
            return;

        const bool hasMech  = (mechAmount > 0.001f);
        const bool hasTube  = (tubeAmount > 0.001f);
        const bool hasBit   = (bitAmount > 0.001f);
        const bool hasXcita = (xcitaAmount > 0.001f);

        if (!hasMech && !hasTube && !hasBit && !hasXcita)
            return;

        // 1. Bitcrusher: applied at BASE sample rate before oversampling
        if (hasBit)
        {
            // Bit depth: 16-bit down to 4-bit
            const float bits = juce::jmap(bitAmount, 16.0f, 4.0f);
            const float levels = std::pow(2.0f, bits);
            const float invLevels = 1.0f / levels;

            // Decimation: 1x to 32x
            const int maxHold = 1 + static_cast<int>(bitAmount * 31.0f);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* data = buffer.getWritePointer(ch);
                int& holdCounter = (ch == 0) ? bitHoldCounterL : bitHoldCounterR;
                float& holdSample = (ch == 0) ? bitHoldSampleL : bitHoldSampleR;

                for (int i = 0; i < numSamples; ++i)
                {
                    if (++holdCounter >= maxHold)
                    {
                        holdCounter = 0;
                        const float in = data[i];
                        holdSample = std::round(in * levels) * invLevels;
                    }
                    data[i] = (1.0f - bitAmount) * data[i] + bitAmount * holdSample;
                }
            }
        }

        // If only bitcrusher was engaged and no continuous analog circuits, we can finish early
        if (!hasMech && !hasTube && !hasXcita)
            return;

        // 2. 4x Oversampling for Mech, Tube, Xcita
        juce::dsp::AudioBlock<float> originalBlock(buffer);
        auto oversampledBlock = oversampling.processSamplesUp(originalBlock);

        const int osNumSamples = static_cast<int>(oversampledBlock.getNumSamples());

        // Exponential drive calculations
        // Mech: 1.0 to 25.0
        const float mechDrive = std::pow(25.0f, mechAmount);
        // Tube: 1.0 to 15.0
        const float tubeDrive = std::pow(15.0f, tubeAmount);
        // Xcita drive: 1.0 to 10.0
        const float xcitaDrive = 1.0f + xcitaAmount * 9.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* osData = oversampledBlock.getChannelPointer(ch);

            auto& preFilt   = (ch == 0) ? mechPreFilterL : mechPreFilterR;
            auto& postFilt  = (ch == 0) ? mechPostFilterL : mechPostFilterR;
            auto& tubeDc    = (ch == 0) ? tubeDcBlockerL : tubeDcBlockerR;
            auto& xcitaHpf  = (ch == 0) ? xcitaHpfL : xcitaHpfR;
            auto& xcitaDc   = (ch == 0) ? xcitaDcBlockerL : xcitaDcBlockerR;

            for (int i = 0; i < osNumSamples; ++i)
            {
                const float dry = osData[i];
                float out = dry;

                // Mech Circuit
                if (hasMech)
                {
                    // Pre-emphasis filter at 1.2 kHz (+6 dB)
                    const float pre = preFilt.processSample(dry);
                    const float dx = mechDrive * pre;
                    float shaped = 0.0f;

                    if (dx >= 0.0f)
                    {
                        shaped = std::tanh(dx) + 0.15f * std::sin(dx * juce::MathConstants<float>::pi);
                    }
                    else
                    {
                        shaped = juce::jlimit(-0.9f, 0.0f, dx * 1.2f);
                    }

                    // Post-de-emphasis filter at 1.2 kHz (-6 dB)
                    const float mechOut = postFilt.processSample(shaped);
                    out += mechOut * (mechAmount * 0.5f);
                }

                // Tube Circuit
                if (hasTube)
                {
                    const float x = dry;
                    const float x2 = x * x;
                    const float rawTube = std::tanh(tubeDrive * x + 0.1f * x2) - std::tanh(0.1f * x2);
                    const float tubeBlocked = tubeDc.processSample(rawTube);
                    out += tubeBlocked * (tubeAmount * 0.6f);
                }

                // Xcita Circuit
                if (hasXcita)
                {
                    const float xHf = xcitaHpf.processSample(dry);
                    const float rectified = 0.7f * xHf + 0.3f * std::abs(xHf);
                    const float excited = std::tanh(xcitaDrive * rectified);
                    const float xcitaBlocked = xcitaDc.processSample(excited);
                    out += xcitaBlocked * (xcitaAmount * 0.4f);
                }

                osData[i] = out;
            }
        }

        // Downsample back to original rate
        oversampling.processSamplesDown(originalBlock);
    }

    [[nodiscard]] int getLatencyInSamples() const noexcept
    {
        return static_cast<int>(oversampling.getLatencyInSamples());
    }

private:
    double baseSampleRate { 44100.0 };

    float mechAmount  { 0.0f };
    float tubeAmount  { 0.0f };
    float bitAmount   { 0.0f };
    float xcitaAmount { 0.0f };
    bool  isBypassed  { false };

    juce::dsp::Oversampling<float> oversampling;

    // Mech filters (oversampled rate)
    juce::dsp::IIR::Filter<float> mechPreFilterL;
    juce::dsp::IIR::Filter<float> mechPreFilterR;
    juce::dsp::IIR::Filter<float> mechPostFilterL;
    juce::dsp::IIR::Filter<float> mechPostFilterR;

    // Tube DC blocker (oversampled rate)
    juce::dsp::IIR::Filter<float> tubeDcBlockerL;
    juce::dsp::IIR::Filter<float> tubeDcBlockerR;

    // Xcita filters (oversampled rate)
    juce::dsp::IIR::Filter<float> xcitaHpfL;
    juce::dsp::IIR::Filter<float> xcitaHpfR;
    juce::dsp::IIR::Filter<float> xcitaDcBlockerL;
    juce::dsp::IIR::Filter<float> xcitaDcBlockerR;

    // Bitcrusher state (base rate)
    float bitHoldSampleL { 0.0f };
    float bitHoldSampleR { 0.0f };
    int   bitHoldCounterL { 0 };
    int   bitHoldCounterR { 0 };
};

} // namespace camelphat::dsp
