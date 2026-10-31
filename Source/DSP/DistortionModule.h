#pragma once

#include "IDSPModule.h"
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace camelphat::dsp {

/**
 * @brief 4-Stage parallel distortion module operating inside 4x minimum-phase polyphase IIR oversampling.
 * Faithfully models CamelPhat 3's Tube, Mech, Bit Crusher, and Xcita algorithms with 10 Hz DC blocking.
 */
class DistortionModule : public IAudioProcessorModule
{
public:
    DistortionModule()
        // 2 channels, factor of 2 (2^2 = 4x oversampling), minimum-phase polyphase IIR (zero latency)
        : oversampling(2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false)
    {
    }

    ~DistortionModule() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        oversampling.initProcessing(spec.maximumBlockSize);

        // Prepare 10 Hz DC blocking filters
        dcBlockerLeft.prepare(spec);
        dcBlockerRight.prepare(spec);

        auto dcCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate, 10.0f);
        dcBlockerLeft.coefficients  = dcCoeffs;
        dcBlockerRight.coefficients = dcCoeffs;

        // Prepare high-band filter for Xcita circuit
        xcitaFilterLeft.prepare(spec);
        xcitaFilterRight.prepare(spec);

        auto xcitaCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate, 3000.0f);
        xcitaFilterLeft.coefficients  = xcitaCoeffs;
        xcitaFilterRight.coefficients = xcitaCoeffs;

        reset();
    }

    void reset() noexcept override
    {
        oversampling.reset();
        dcBlockerLeft.reset();
        dcBlockerRight.reset();
        xcitaFilterLeft.reset();
        xcitaFilterRight.reset();

        bitHoldSampleL = 0.0f;
        bitHoldSampleR = 0.0f;
        bitHoldCounterL = 0;
        bitHoldCounterR = 0;
    }

    void setParameters(float tube, float mech, float bit, float xcita, float mix) noexcept
    {
        tubeAmount  = juce::jlimit(0.0f, 1.0f, tube);
        mechAmount  = juce::jlimit(0.0f, 1.0f, mech);
        bitAmount   = juce::jlimit(0.0f, 1.0f, bit);
        xcitaAmount = juce::jlimit(0.0f, 1.0f, xcita);
        distMix     = juce::jlimit(0.0f, 1.0f, mix);
    }

    void process(juce::AudioBuffer<float>& buffer) noexcept override
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(2, buffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0)
            return;

        // If no distortion is engaged and mix is neutral, skip heavy processing
        const bool isEngaged = (tubeAmount > 0.001f || mechAmount > 0.001f ||
                                bitAmount > 0.001f || xcitaAmount > 0.001f);

        if (!isEngaged && distMix >= 0.999f)
            return;

        // Upsample 4x via minimum-phase polyphase IIR (zero latency)
        juce::dsp::AudioBlock<float> originalBlock(buffer);
        auto oversampledBlock = oversampling.processSamplesUp(originalBlock);

        const int osNumSamples = static_cast<int>(oversampledBlock.getNumSamples());
        float* osLeft = oversampledBlock.getChannelPointer(0);
        float* osRight = numChannels > 1 ? oversampledBlock.getChannelPointer(1) : osLeft;

        // Process oversampled stereo stream
        for (int i = 0; i < osNumSamples; ++i)
        {
            osLeft[i]  = processSampleChannel(osLeft[i], 0);
            if (numChannels > 1)
                osRight[i] = processSampleChannel(osRight[i], 1);
        }

        // Downsample back to original rate
        oversampling.processSamplesDown(originalBlock);

        // Apply 10 Hz DC blocking highpass filter to remove any DC offset from asymmetric clipping
        auto* lPtr = buffer.getWritePointer(0);
        for (int i = 0; i < numSamples; ++i)
            lPtr[i] = dcBlockerLeft.processSample(lPtr[i]);

        if (numChannels > 1)
        {
            auto* rPtr = buffer.getWritePointer(1);
            for (int i = 0; i < numSamples; ++i)
                rPtr[i] = dcBlockerRight.processSample(rPtr[i]);
        }
    }

private:
    [[nodiscard]] inline float processSampleChannel(float input, int channel) noexcept
    {
        float sample = input;
        float wet = 0.0f;
        int activeStages = 0;

        // 1. Tube Circuit: Warm analog saturation (smooth hyperbolic tangent with even harmonics)
        if (tubeAmount > 0.001f)
        {
            const float drive = 1.0f + tubeAmount * 8.0f;
            const float driven = sample * drive;
            // Soft saturation with mild asymmetry for even-order harmonic warmth
            const float saturated = std::tanh(driven + 0.05f * driven * driven);
            wet += saturated * (1.0f / (1.0f + 0.5f * tubeAmount));
            activeStages++;
        }

        // 2. Mech Circuit: Harsh, industrial metallic clipping
        if (mechAmount > 0.001f)
        {
            const float drive = 1.0f + mechAmount * 12.0f;
            float x = sample * drive;
            // Asymmetric diode-style hard-knee clipping
            if (x > 0.8f)
                x = 0.8f + (x - 0.8f) / (1.0f + (x - 0.8f) * (x - 0.8f));
            else if (x < -0.6f)
                x = -0.6f + (x + 0.6f) / (1.0f + (x + 0.6f) * (x + 0.6f));
            wet += x;
            activeStages++;
        }

        // 3. Bit Crusher: Quantization & sample-hold decimation
        if (bitAmount > 0.001f)
        {
            // Decimation hold
            int& holdCounter = (channel == 0) ? bitHoldCounterL : bitHoldCounterR;
            float& holdSample = (channel == 0) ? bitHoldSampleL : bitHoldSampleR;

            const int maxHold = 1 + static_cast<int>(bitAmount * 12.0f);
            if (++holdCounter >= maxHold)
            {
                holdCounter = 0;
                // Quantization: 16 bits down to 3 bits
                const float bits = juce::jmap(bitAmount, 16.0f, 3.5f);
                const float levels = std::pow(2.0f, bits);
                holdSample = std::round(sample * levels) / levels;
            }
            wet += holdSample;
            activeStages++;
        }

        // 4. Xcita Circuit: Dynamic high-frequency harmonic exciter
        if (xcitaAmount > 0.001f)
        {
            auto& xFilter = (channel == 0) ? xcitaFilterLeft : xcitaFilterRight;
            const float highBand = xFilter.processSample(sample);
            const float drive = 1.0f + xcitaAmount * 6.0f;
            const float excited = std::tanh(highBand * drive);
            wet += excited * xcitaAmount;
            activeStages++;
        }

        if (activeStages > 0)
        {
            const float combinedWet = wet / static_cast<float>(std::max(1, activeStages));
            return (1.0f - distMix) * input + distMix * combinedWet;
        }

        return input;
    }

    double sampleRate { 44100.0 };

    float tubeAmount  { 0.0f };
    float mechAmount  { 0.0f };
    float bitAmount   { 0.0f };
    float xcitaAmount { 0.0f };
    float distMix     { 1.0f };

    juce::dsp::Oversampling<float> oversampling;

    juce::dsp::IIR::Filter<float> dcBlockerLeft;
    juce::dsp::IIR::Filter<float> dcBlockerRight;

    juce::dsp::IIR::Filter<float> xcitaFilterLeft;
    juce::dsp::IIR::Filter<float> xcitaFilterRight;

    float bitHoldSampleL { 0.0f };
    float bitHoldSampleR { 0.0f };
    int   bitHoldCounterL { 0 };
    int   bitHoldCounterR { 0 };
};

} // namespace camelphat::dsp
