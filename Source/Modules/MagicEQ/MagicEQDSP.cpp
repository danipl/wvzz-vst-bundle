#include "MagicEQDSP.h"

namespace wvzz::modules {

MagicEQDSP::MagicEQDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void MagicEQDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Magic EQ Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("amount"), 1 }, "Magic EQ Amount", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("tune"), 1 }, "Magic EQ Tune", 60.0f, 120.0f, 80.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("phat"), 1 }, "Magic EQ Phat Mode", false));
}

void MagicEQDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam = apvts.getRawParameterValue(paramId("bypass"));
    amountParam = apvts.getRawParameterValue(paramId("amount"));
    tuneParam   = apvts.getRawParameterValue(paramId("tune"));
    phatParam   = apvts.getRawParameterValue(paramId("phat"));
}

void MagicEQDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    highShelfL.prepare(spec);
    highShelfR.prepare(spec);

    const double rampTimeSec = 0.015;
    amountSmoothed.reset(sampleRate, rampTimeSec);
    tuneSmoothed.reset(sampleRate, rampTimeSec);

    if (amountParam != nullptr) amountSmoothed.setCurrentAndTargetValue(amountParam->load(std::memory_order_relaxed));
    if (tuneParam != nullptr)   tuneSmoothed.setCurrentAndTargetValue(tuneParam->load(std::memory_order_relaxed));

    reset();
}

void MagicEQDSP::reset() noexcept
{
    boostS1L = 0.0f; boostS2L = 0.0f;
    boostS1R = 0.0f; boostS2R = 0.0f;

    dipS1L = 0.0f; dipS2L = 0.0f;
    dipS1R = 0.0f; dipS2R = 0.0f;

    highShelfL.reset();
    highShelfR.reset();
}

void MagicEQDSP::setParameters(float amountNorm, float tuneHz, bool phatModeEnabled, bool bypass) noexcept
{
    amountSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, amountNorm));
    tuneSmoothed.setCurrentAndTargetValue(juce::jlimit(60.0f, 120.0f, tuneHz));
    if (phatParam != nullptr)
        phatParam->store(phatModeEnabled ? 1.0f : 0.0f, std::memory_order_relaxed);
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void MagicEQDSP::updateCoefficients(float amountVal, float tuneVal) noexcept
{
    if (sampleRate <= 0.0)
        return;

    const float fs = static_cast<float>(sampleRate);

    // 1. Peaking Boost at tune Hz
    {
        const float boostDb = amountVal * 12.0f;
        const float A = std::pow(10.0f, boostDb / 40.0f);
        const float w0 = 2.0f * juce::MathConstants<float>::pi * tuneVal / fs;
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

    // 2. Complimentary Dip at 2 * tune Hz
    {
        const float dipFreq = juce::jmin(fs * 0.48f, 2.0f * tuneVal);
        const float dipDb = -2.5f * amountVal;
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

void MagicEQDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
        return;

    if (amountParam != nullptr) amountSmoothed.setTargetValue(amountParam->load(std::memory_order_relaxed));
    if (tuneParam != nullptr)   tuneSmoothed.setTargetValue(tuneParam->load(std::memory_order_relaxed));

    const float curAmount = amountSmoothed.skip(numSamples);
    const float curTune   = tuneSmoothed.skip(numSamples);

    const float modAmount = juce::jlimit(0.0f, 1.0f, curAmount + ctx.getModulationOffset(ModTarget::MagicEQAmount));
    const float modTune   = juce::jlimit(60.0f, 120.0f, curTune + ctx.getModulationOffset(ModTarget::MagicEQTune) * 30.0f);

    if (modAmount <= 0.001f)
        return;

    const bool phatMode = (phatParam != nullptr) && (phatParam->load(std::memory_order_relaxed) > 0.5f);

    updateCoefficients(modAmount, modTune);

    float* channelLeft  = ctx.mainBuffer.getWritePointer(0);
    float* channelRight = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        // Left Channel
        const float inL = channelLeft[i];
        float boostOutL = b0_boost * inL + boostS1L;
        if (phatMode)
        {
            boostS1L = b1_boost * inL - a1_boost * std::tanh(boostOutL * 1.2f) + boostS2L;
            boostS2L = b2_boost * inL - a2_boost * std::tanh(boostOutL * 1.2f);
        }
        else
        {
            boostS1L = b1_boost * inL - a1_boost * boostOutL + boostS2L;
            boostS2L = b2_boost * inL - a2_boost * boostOutL;
        }

        float outL = b0_dip * boostOutL + dipS1L;
        dipS1L = b1_dip * boostOutL - a1_dip * outL + dipS2L;
        dipS2L = b2_dip * boostOutL - a2_dip * outL;
        channelLeft[i] = outL;

        // Right Channel
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

    if (phatMode && modAmount > 0.01f)
    {
        auto shelfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, 8000.0f, 0.7071f, juce::Decibels::decibelsToGain(3.0f * modAmount));
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

void MagicEQDSP::randomize(juce::Random& rng)
{
    if (amountSmoothed.isSmoothing()) amountSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.7f);
    if (tuneSmoothed.isSmoothing())   tuneSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 50.0f + 65.0f);
}

} // namespace wvzz::modules
