#pragma once

#include "IDSPModule.h"
#include <juce_dsp/juce_dsp.h>

namespace camelphat::dsp {

/**
 * @brief Dual-resonant band-pass filter and band-reject splitter.
 * Uses Topology-Preserving Transform (TPT) State Variable Filters for unconditional stability.
 * Recreates the input splitting topology of CamelPhat 3.
 */
class BPFilter : public IDSPModule
{
public:
    BPFilter() = default;
    ~BPFilter() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;

        hpfLeft.prepare(spec);
        hpfRight.prepare(spec);
        lpfLeft.prepare(spec);
        lpfRight.prepare(spec);

        hpfLeft.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        hpfRight.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        lpfLeft.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        lpfRight.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

        reset();
    }

    void reset() noexcept override
    {
        hpfLeft.reset();
        hpfRight.reset();
        lpfLeft.reset();
        lpfRight.reset();
    }

    /**
     * @brief Splits input stereo audio into passband and band-reject buffers.
     * @param inputBuffer Read-only source audio.
     * @param passbandBuffer Destination for bandpass filtered audio.
     * @param bandRejectBuffer Destination for out-of-band rejected audio.
     * @param cutoffLow High-pass cutoff frequency in Hz (20 - 20000).
     * @param cutoffHigh Low-pass cutoff frequency in Hz (20 - 20000).
     * @param resonance Q resonance factor (0.0 to 1.0).
     * @param bpMix Wet/dry blend for the band-pass filter (0.0 to 1.0).
     */
    void process(const juce::AudioBuffer<float>& inputBuffer,
                 juce::AudioBuffer<float>& passbandBuffer,
                 juce::AudioBuffer<float>& bandRejectBuffer,
                 float cutoffLow,
                 float cutoffHigh,
                 float resonance,
                 float bpMix) noexcept
    {
        const int numSamples = inputBuffer.getNumSamples();
        const int numChannels = juce::jmin(2, inputBuffer.getNumChannels(), passbandBuffer.getNumChannels(), bandRejectBuffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0)
            return;

        // Map resonance [0, 1] to TPT Q range [0.5, 10.0]
        const float q = juce::jmap(juce::jlimit(0.0f, 1.0f, resonance), 0.5f, 10.0f);

        // Clamp frequencies safely within Nyquist limit
        const float maxFreq = static_cast<float>(sampleRate * 0.49);
        const float safeLow  = juce::jlimit(20.0f, maxFreq, cutoffLow);
        const float safeHigh = juce::jlimit(20.0f, maxFreq, cutoffHigh);

        hpfLeft.setCutoffFrequency(safeLow);
        hpfLeft.setResonance(q);
        hpfRight.setCutoffFrequency(safeLow);
        hpfRight.setResonance(q);

        lpfLeft.setCutoffFrequency(safeHigh);
        lpfLeft.setResonance(q);
        lpfRight.setCutoffFrequency(safeHigh);
        lpfRight.setResonance(q);

        // Copy input to passband and bandReject
        for (int ch = 0; ch < numChannels; ++ch)
        {
            passbandBuffer.copyFrom(ch, 0, inputBuffer.getReadPointer(ch), numSamples);
            bandRejectBuffer.copyFrom(ch, 0, inputBuffer.getReadPointer(ch), numSamples);
        }

        // Process Left Channel
        if (numChannels >= 1)
        {
            float* passL = passbandBuffer.getWritePointer(0);
            const float* inL = inputBuffer.getReadPointer(0);
            float* rejL = bandRejectBuffer.getWritePointer(0);

            for (int i = 0; i < numSamples; ++i)
            {
                const float in = inL[i];
                float filtered = hpfLeft.processSample(0, in);
                filtered = lpfLeft.processSample(0, filtered);

                const float wet = (1.0f - bpMix) * in + bpMix * filtered;
                passL[i] = wet;
                rejL[i] = in - wet;
            }
        }

        // Process Right Channel
        if (numChannels >= 2)
        {
            float* passR = passbandBuffer.getWritePointer(1);
            const float* inR = inputBuffer.getReadPointer(1);
            float* rejR = bandRejectBuffer.getWritePointer(1);

            for (int i = 0; i < numSamples; ++i)
            {
                const float in = inR[i];
                float filtered = hpfRight.processSample(0, in);
                filtered = lpfRight.processSample(0, filtered);

                const float wet = (1.0f - bpMix) * in + bpMix * filtered;
                passR[i] = wet;
                rejR[i] = in - wet;
            }
        }
    }

private:
    double sampleRate { 44100.0 };

    juce::dsp::StateVariableTPTFilter<float> hpfLeft;
    juce::dsp::StateVariableTPTFilter<float> hpfRight;
    juce::dsp::StateVariableTPTFilter<float> lpfLeft;
    juce::dsp::StateVariableTPTFilter<float> lpfRight;
};

} // namespace camelphat::dsp
