#include "CompressorDSP.h"

namespace wvzz::modules {

CompressorDSP::CompressorDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void CompressorDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto compRelRange = juce::NormalisableRange<float>(10.0f, 1500.0f, 1.0f, 0.35f);

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Compressor Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("amount"), 1 }, "Compressor Amount", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("release"), 1 }, "Compressor Release", compRelRange, 120.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("phat"), 1 }, "Compressor Phat Mode", false));
}

void CompressorDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam  = apvts.getRawParameterValue(paramId("bypass"));
    amountParam  = apvts.getRawParameterValue(paramId("amount"));
    releaseParam = apvts.getRawParameterValue(paramId("release"));
    phatParam    = apvts.getRawParameterValue(paramId("phat"));
}

void CompressorDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const double rampTimeSec = 0.015;
    amountSmoothed.reset(sampleRate, rampTimeSec);
    releaseSmoothed.reset(sampleRate, rampTimeSec);

    if (amountParam != nullptr)  amountSmoothed.setCurrentAndTargetValue(amountParam->load(std::memory_order_relaxed));
    if (releaseParam != nullptr) releaseSmoothed.setCurrentAndTargetValue(releaseParam->load(std::memory_order_relaxed));

    reset();
}

void CompressorDSP::reset() noexcept
{
    envelope = 0.0f;
    gainReductionDb.store(0.0f, std::memory_order_relaxed);
}

void CompressorDSP::setParameters(float amountNorm, float releaseMs, bool phatModeEnabled, bool bypass) noexcept
{
    amountSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, amountNorm));
    releaseSmoothed.setCurrentAndTargetValue(juce::jlimit(10.0f, 1500.0f, releaseMs));
    if (phatParam != nullptr)
        phatParam->store(phatModeEnabled ? 1.0f : 0.0f, std::memory_order_relaxed);
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void CompressorDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
    {
        gainReductionDb.store(0.0f, std::memory_order_relaxed);
        return;
    }

    if (amountParam != nullptr)  amountSmoothed.setTargetValue(amountParam->load(std::memory_order_relaxed));
    if (releaseParam != nullptr) releaseSmoothed.setTargetValue(releaseParam->load(std::memory_order_relaxed));

    const float curAmount  = amountSmoothed.skip(numSamples);
    const float curRelease = releaseSmoothed.skip(numSamples);

    const float modAmount = juce::jlimit(0.0f, 1.0f, curAmount + ctx.getModulationOffset(ModTarget::CompAmount));
    if (modAmount <= 0.001f)
    {
        gainReductionDb.store(0.0f, std::memory_order_relaxed);
        return;
    }

    const bool phatMode = (phatParam != nullptr) && (phatParam->load(std::memory_order_relaxed) > 0.5f);

    const float thresholdDb = -36.0f * modAmount;
    const float autoMakeupDb = 14.0f * modAmount;
    const float autoMakeupLinear = juce::Decibels::decibelsToGain(autoMakeupDb);

    float attackMs, ratio, kneeWidthDb;
    if (phatMode)
    {
        attackMs = 0.8f;
        ratio = 12.0f;
        kneeWidthDb = 1.0f;
    }
    else
    {
        attackMs = 3.5f;
        ratio = 3.5f;
        kneeWidthDb = 6.0f;
    }

    const float safeAttack = juce::jmax(0.05f, attackMs);
    const float safeRelease = juce::jmax(1.0f, curRelease);

    const float attackCoeff  = std::exp(-1.0f / (static_cast<float>(sampleRate) * safeAttack * 0.001f));
    const float releaseCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * safeRelease * 0.001f));

    const float halfKnee = kneeWidthDb * 0.5f;
    const float slope = 1.0f - (1.0f / ratio);

    constexpr float ceilingLinear = 0.988553f; // -0.1 dBFS
    const float invCeiling = 1.0f / ceilingLinear;

    float* channelLeft  = ctx.mainBuffer.getWritePointer(0);
    float* channelRight = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = channelLeft[i];
        const float inR = (channelRight != nullptr) ? channelRight[i] : inL;

        const float detectorIn = juce::jmax(std::abs(inL), std::abs(inR));

        if (detectorIn > envelope)
            envelope = detectorIn + attackCoeff * (envelope - detectorIn);
        else
            envelope = detectorIn + releaseCoeff * (envelope - detectorIn);

        const float envDb = juce::Decibels::gainToDecibels(std::max(envelope, 1.0e-5f));

        float grDb = 0.0f;
        if (envDb > thresholdDb + halfKnee)
        {
            grDb = -slope * (envDb - thresholdDb);
        }
        else if (envDb > thresholdDb - halfKnee)
        {
            const float diff = envDb - thresholdDb + halfKnee;
            grDb = -slope * (diff * diff) / (2.0f * kneeWidthDb);
        }

        gainReductionDb.store(grDb, std::memory_order_relaxed);
        const float compGain = juce::Decibels::decibelsToGain(grDb) * autoMakeupLinear;

        float outL = inL * compGain;
        float outR = inR * compGain;

        if (phatMode)
        {
            outL = std::tanh(outL * invCeiling) * ceilingLinear;
            outR = std::tanh(outR * invCeiling) * ceilingLinear;
        }

        channelLeft[i] = outL;
        if (channelRight != nullptr)
            channelRight[i] = outR;
    }
}

void CompressorDSP::randomize(juce::Random& rng)
{
    if (amountSmoothed.isSmoothing())  amountSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.7f);
    if (releaseSmoothed.isSmoothing()) releaseSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 400.0f + 50.0f);
}

} // namespace wvzz::modules
