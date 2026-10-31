#pragma once

#include "IDSPModule.h"
#include "EnvelopeFollower.h"
#include <juce_dsp/juce_dsp.h>

namespace camelphat::dsp {

/**
 * @brief Resonant Multi-Mode Filter (Lowpass, Bandpass, Highpass) with Envelope Follower modulation.
 * Uses TPT State Variable Filter structures for analog sound and guaranteed stability under modulation.
 */
class MMFilter : public IAudioProcessorModule
{
public:
    enum class FilterMode
    {
        Lowpass = 0,
        Bandpass,
        Highpass
    };

    MMFilter() = default;
    ~MMFilter() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;

        filterLeft.prepare(spec);
        filterRight.prepare(spec);

        envFollower.prepare(spec.sampleRate);
        reset();
    }

    void reset() noexcept override
    {
        filterLeft.reset();
        filterRight.reset();
        envFollower.reset();
    }

    void setParameters(float cutoffHz,
                       float resonanceNorm,
                       int modeIndex,
                       float envAmountNorm,
                       float envAttackMs,
                       float envReleaseMs) noexcept
    {
        baseCutoff = juce::jlimit(20.0f, static_cast<float>(sampleRate * 0.49), cutoffHz);
        resonance = juce::jlimit(0.0f, 1.0f, resonanceNorm);

        switch (modeIndex)
        {
            case 1:  currentMode = FilterMode::Bandpass; break;
            case 2:  currentMode = FilterMode::Highpass; break;
            default: currentMode = FilterMode::Lowpass;  break;
        }

        envAmount = juce::jlimit(-1.0f, 1.0f, envAmountNorm);
        envFollower.setAttackRelease(envAttackMs, envReleaseMs);

        // Map filter type
        switch (currentMode)
        {
            case FilterMode::Lowpass:
                filterLeft.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
                filterRight.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
                break;
            case FilterMode::Bandpass:
                filterLeft.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
                filterRight.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
                break;
            case FilterMode::Highpass:
                filterLeft.setType(juce::dsp::StateVariableTPTFilterType::highpass);
                filterRight.setType(juce::dsp::StateVariableTPTFilterType::highpass);
                break;
        }

        // Map resonance [0, 1] to TPT Q [0.5, 12.0]
        const float q = juce::jmap(resonance, 0.5f, 12.0f);
        filterLeft.setResonance(q);
        filterRight.setResonance(q);
    }

    void process(juce::AudioBuffer<float>& buffer) noexcept override
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = juce::jmin(2, buffer.getNumChannels());

        if (numSamples == 0 || numChannels == 0)
            return;

        float* channelLeft  = buffer.getWritePointer(0);
        float* channelRight = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

        const float maxFreq = static_cast<float>(sampleRate * 0.49);

        for (int i = 0; i < numSamples; ++i)
        {
            const float inL = channelLeft[i];
            const float inR = channelRight != nullptr ? channelRight[i] : inL;

            // Envelope detection on combined mono sum
            const float env = envFollower.processSample(0.5f * (inL + inR));

            // Dynamic frequency modulation (up to +/- 4 octaves)
            float modulatedFreq = baseCutoff;
            if (std::abs(envAmount) > 0.001f)
            {
                const float octaveShift = envAmount * env * 4.0f;
                modulatedFreq *= std::pow(2.0f, octaveShift);
                modulatedFreq = juce::jlimit(20.0f, maxFreq, modulatedFreq);
            }

            filterLeft.setCutoffFrequency(modulatedFreq);
            channelLeft[i] = filterLeft.processSample(0, inL);

            if (channelRight != nullptr)
            {
                filterRight.setCutoffFrequency(modulatedFreq);
                channelRight[i] = filterRight.processSample(0, inR);
            }
        }
    }

private:
    double sampleRate { 44100.0 };

    float baseCutoff { 1000.0f };
    float resonance  { 0.2f };
    float envAmount  { 0.0f };
    FilterMode currentMode { FilterMode::Lowpass };

    juce::dsp::StateVariableTPTFilter<float> filterLeft;
    juce::dsp::StateVariableTPTFilter<float> filterRight;
    EnvelopeFollower envFollower;
};

} // namespace camelphat::dsp
