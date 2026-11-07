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
        juce::ParameterID{ paramId("tune"), 1 }, "Magic EQ Tune", 45.0f, 180.0f, 80.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("phat"), 1 }, "Magic EQ Phat Mode", false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("pre"), 1 }, "Magic EQ Pre Dist", false));
}

void MagicEQDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam = apvts.getRawParameterValue(paramId("bypass"));
    amountParam = apvts.getRawParameterValue(paramId("amount"));
    tuneParam   = apvts.getRawParameterValue(paramId("tune"));
    phatParam   = apvts.getRawParameterValue(paramId("phat"));
    preParam    = apvts.getRawParameterValue(paramId("pre"));
}

void MagicEQDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const double rampTimeSec = 0.015;
    amountSmoothed.reset(sampleRate, rampTimeSec);
    tuneSmoothed.reset(sampleRate, rampTimeSec);

    if (amountParam != nullptr) amountSmoothed.setCurrentAndTargetValue(amountParam->load(std::memory_order_relaxed));
    if (tuneParam != nullptr)   tuneSmoothed.setCurrentAndTargetValue(tuneParam->load(std::memory_order_relaxed));

    reset();
}

void MagicEQDSP::reset() noexcept
{
    boostS1L = 0.0; boostS2L = 0.0;
    boostS1R = 0.0; boostS2R = 0.0;

    dipS1L = 0.0; dipS2L = 0.0;
    dipS1R = 0.0; dipS2R = 0.0;

    shelfS1L = 0.0; shelfS2L = 0.0;
    shelfS1R = 0.0; shelfS2R = 0.0;
}

void MagicEQDSP::setParameters(float amountNorm, float tuneHz, bool phatModeEnabled, bool bypass, bool pre) noexcept
{
    amountSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, amountNorm));
    tuneSmoothed.setCurrentAndTargetValue(juce::jlimit(45.0f, 180.0f, tuneHz));
    if (phatParam != nullptr)
        phatParam->store(phatModeEnabled ? 1.0f : 0.0f, std::memory_order_relaxed);
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
    if (preParam != nullptr)
        preParam->store(pre ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void MagicEQDSP::updateCoefficients(float amountVal, float tuneVal) noexcept
{
    if (sampleRate <= 0.0)
        return;

    const double fs = sampleRate;
    const double amt = static_cast<double>(amountVal);
    const double tune = static_cast<double>(tuneVal);

    // 1. Peaking Boost at tune Hz
    {
        const double boostDb = amt * 12.0;
        const double A = std::pow(10.0, boostDb / 40.0);
        const double w0 = 2.0 * juce::MathConstants<double>::pi * tune / fs;
        const double cosW0 = std::cos(w0);
        const double sinW0 = std::sin(w0);
        constexpr double q = 1.6;
        const double alpha = sinW0 / (2.0 * q);

        const double a0 = 1.0 + alpha / A;
        b0_boost = (1.0 + alpha * A) / a0;
        b1_boost = (-2.0 * cosW0) / a0;
        b2_boost = (1.0 - alpha * A) / a0;
        a1_boost = (-2.0 * cosW0) / a0;
        a2_boost = (1.0 - alpha / A) / a0;
    }

    // 2. Complimentary Dip at 2 * tune Hz
    {
        const double dipFreq = std::min(fs * 0.45, 2.0 * tune);
        const double dipDb = -2.5 * amt;
        const double A = std::pow(10.0, dipDb / 40.0);
        const double w0 = 2.0 * juce::MathConstants<double>::pi * dipFreq / fs;
        const double cosW0 = std::cos(w0);
        const double sinW0 = std::sin(w0);
        constexpr double q = 2.0;
        const double alpha = sinW0 / (2.0 * q);

        const double a0 = 1.0 + alpha / A;
        b0_dip = (1.0 + alpha * A) / a0;
        b1_dip = (-2.0 * cosW0) / a0;
        b2_dip = (1.0 - alpha * A) / a0;
        a1_dip = (-2.0 * cosW0) / a0;
        a2_dip = (1.0 - alpha / A) / a0;
    }

    // 3. Zero-allocation 8 kHz High Shelf for Phat mode
    {
        const double shelfFreq = std::min(fs * 0.42, 8000.0);
        const double shelfDb = 3.0 * amt;
        const double A = std::pow(10.0, shelfDb / 40.0);
        const double w0 = 2.0 * juce::MathConstants<double>::pi * shelfFreq / fs;
        const double cosW0 = std::cos(w0);
        const double sinW0 = std::sin(w0);
        constexpr double q = 0.70710678;
        const double alpha = sinW0 / (2.0 * q);
        const double twoSqrtAAlpha = 2.0 * std::sqrt(A) * alpha;

        const double a0 = (A + 1.0) - (A - 1.0) * cosW0 + twoSqrtAAlpha;
        b0_shelf = (A * ((A + 1.0) + (A - 1.0) * cosW0 + twoSqrtAAlpha)) / a0;
        b1_shelf = (-2.0 * A * ((A - 1.0) + (A + 1.0) * cosW0)) / a0;
        b2_shelf = (A * ((A + 1.0) + (A - 1.0) * cosW0 - twoSqrtAAlpha)) / a0;
        a1_shelf = (2.0 * ((A - 1.0) - (A + 1.0) * cosW0)) / a0;
        a2_shelf = ((A + 1.0) - (A - 1.0) * cosW0 - twoSqrtAAlpha) / a0;
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
    const float modTune   = juce::jlimit(45.0f, 180.0f, curTune + ctx.getModulationOffset(ModTarget::MagicEQTune) * 45.0f);

    if (modAmount <= 0.001f)
        return;

    const bool phatMode = (phatParam != nullptr) && (phatParam->load(std::memory_order_relaxed) > 0.5f);

    updateCoefficients(modAmount, modTune);

    float* channelLeft  = ctx.mainBuffer.getWritePointer(0);
    float* channelRight = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        // Left Channel: strictly linear biquad recurrence so poles never shift or intermodulate highs
        const double inL = static_cast<double>(channelLeft[i]);
        const double linearBoostL = b0_boost * inL + boostS1L;
        boostS1L = b1_boost * inL - a1_boost * linearBoostL + boostS2L;
        boostS2L = b2_boost * inL - a2_boost * linearBoostL;

        // Saturate only the isolated sub-bass peak band (linearBoostL - inL) in Phat mode
        const double subBandL = linearBoostL - inL;
        const double boostOutL = phatMode
            ? (inL + std::tanh(subBandL * 1.4) * (1.0 / 1.4))
            : linearBoostL;

        double outL = b0_dip * boostOutL + dipS1L;
        dipS1L = b1_dip * boostOutL - a1_dip * outL + dipS2L;
        dipS2L = b2_dip * boostOutL - a2_dip * outL;

        if (phatMode)
        {
            const double shelfInL = outL;
            outL = b0_shelf * shelfInL + shelfS1L;
            shelfS1L = b1_shelf * shelfInL - a1_shelf * outL + shelfS2L;
            shelfS2L = b2_shelf * shelfInL - a2_shelf * outL;
        }
        channelLeft[i] = static_cast<float>(outL);

        // Right Channel
        if (channelRight != nullptr)
        {
            const double inR = static_cast<double>(channelRight[i]);
            const double linearBoostR = b0_boost * inR + boostS1R;
            boostS1R = b1_boost * inR - a1_boost * linearBoostR + boostS2R;
            boostS2R = b2_boost * inR - a2_boost * linearBoostR;

            const double subBandR = linearBoostR - inR;
            const double boostOutR = phatMode
                ? (inR + std::tanh(subBandR * 1.4) * (1.0 / 1.4))
                : linearBoostR;

            double outR = b0_dip * boostOutR + dipS1R;
            dipS1R = b1_dip * boostOutR - a1_dip * outR + dipS2R;
            dipS2R = b2_dip * boostOutR - a2_dip * outR;

            if (phatMode)
            {
                const double shelfInR = outR;
                outR = b0_shelf * shelfInR + shelfS1R;
                shelfS1R = b1_shelf * shelfInR - a1_shelf * outR + shelfS2R;
                shelfS2R = b2_shelf * shelfInR - a2_shelf * outR;
            }
            channelRight[i] = static_cast<float>(outR);
        }
    }
}

void MagicEQDSP::randomize(juce::Random& rng)
{
    if (amountSmoothed.isSmoothing()) amountSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.7f);
    if (tuneSmoothed.isSmoothing())   tuneSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 50.0f + 65.0f);
}

} // namespace wvzz::modules
