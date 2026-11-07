#include "CompressorDSP.h"

namespace wvzz::modules {

CompressorDSP::CompressorDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void CompressorDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto compRelRange   = juce::NormalisableRange<float>(10.0f, 1500.0f, 1.0f, 0.35f);
    auto compRatioRange = juce::NormalisableRange<float>(1.5f, 20.0f, 0.1f, 0.5f);
    auto compAttRange   = juce::NormalisableRange<float>(0.1f, 50.0f, 0.1f, 0.4f);

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Compressor Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("amount"), 1 }, "Compressor Amount", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("release"), 1 }, "Compressor Release", compRelRange, 120.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("phat"), 1 }, "Compressor Phat Mode", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("thresh"), 1 }, "Compressor Threshold", -48.0f, 0.0f, -18.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("ratio"), 1 }, "Compressor Ratio", compRatioRange, 4.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("attack"), 1 }, "Compressor Attack", compAttRange, 5.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("mix"), 1 }, "Compressor Mix", 0.0f, 1.0f, 1.0f));
}

void CompressorDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam  = apvts.getRawParameterValue(paramId("bypass"));
    amountParam  = apvts.getRawParameterValue(paramId("amount"));
    releaseParam = apvts.getRawParameterValue(paramId("release"));
    phatParam    = apvts.getRawParameterValue(paramId("phat"));
    threshParam  = apvts.getRawParameterValue(paramId("thresh"));
    ratioParam   = apvts.getRawParameterValue(paramId("ratio"));
    attackParam  = apvts.getRawParameterValue(paramId("attack"));
    mixParam     = apvts.getRawParameterValue(paramId("mix"));
}

void CompressorDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const double rampTimeSec = 0.015;
    amountSmoothed.reset(sampleRate, rampTimeSec);
    releaseSmoothed.reset(sampleRate, rampTimeSec);
    threshSmoothed.reset(sampleRate, rampTimeSec);
    ratioSmoothed.reset(sampleRate, rampTimeSec);
    attackSmoothed.reset(sampleRate, rampTimeSec);
    mixSmoothed.reset(sampleRate, rampTimeSec);

    if (amountParam != nullptr)  amountSmoothed.setCurrentAndTargetValue(amountParam->load(std::memory_order_relaxed));
    if (releaseParam != nullptr) releaseSmoothed.setCurrentAndTargetValue(releaseParam->load(std::memory_order_relaxed));
    if (threshParam != nullptr)  threshSmoothed.setCurrentAndTargetValue(threshParam->load(std::memory_order_relaxed));
    if (ratioParam != nullptr)   ratioSmoothed.setCurrentAndTargetValue(ratioParam->load(std::memory_order_relaxed));
    if (attackParam != nullptr)  attackSmoothed.setCurrentAndTargetValue(attackParam->load(std::memory_order_relaxed));
    if (mixParam != nullptr)     mixSmoothed.setCurrentAndTargetValue(mixParam->load(std::memory_order_relaxed));

    reset();
}

void CompressorDSP::reset() noexcept
{
    fastEnvelopeDb = 0.0f;
    slowEnvelopeDb = 0.0f;
    prevInNormL    = 0.0f;
    prevInNormR    = 0.0f;
    gainReductionDb.store(0.0f, std::memory_order_relaxed);
    saturationActivity.store(0.0f, std::memory_order_relaxed);
}

void CompressorDSP::setParameters(float amountNorm, float releaseMs, bool phatModeEnabled, bool bypass,
                                  float threshDb, float ratioVal, float attackMsVal, float mixVal) noexcept
{
    amountSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, amountNorm));
    releaseSmoothed.setCurrentAndTargetValue(juce::jlimit(10.0f, 1500.0f, releaseMs));
    threshSmoothed.setCurrentAndTargetValue(juce::jlimit(-48.0f, 0.0f, threshDb));
    ratioSmoothed.setCurrentAndTargetValue(juce::jlimit(1.5f, 20.0f, ratioVal));
    attackSmoothed.setCurrentAndTargetValue(juce::jlimit(0.1f, 50.0f, attackMsVal));
    mixSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, mixVal));

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
        fastEnvelopeDb = 0.0f;
        slowEnvelopeDb = 0.0f;
        prevInNormL    = 0.0f;
        prevInNormR    = 0.0f;
        gainReductionDb.store(0.0f, std::memory_order_relaxed);
        saturationActivity.store(0.0f, std::memory_order_relaxed);
        return;
    }

    if (amountParam != nullptr)  amountSmoothed.setTargetValue(amountParam->load(std::memory_order_relaxed));
    if (releaseParam != nullptr) releaseSmoothed.setTargetValue(releaseParam->load(std::memory_order_relaxed));
    if (threshParam != nullptr)  threshSmoothed.setTargetValue(threshParam->load(std::memory_order_relaxed));
    if (ratioParam != nullptr)   ratioSmoothed.setTargetValue(ratioParam->load(std::memory_order_relaxed));
    if (attackParam != nullptr)  attackSmoothed.setTargetValue(attackParam->load(std::memory_order_relaxed));
    if (mixParam != nullptr)     mixSmoothed.setTargetValue(mixParam->load(std::memory_order_relaxed));

    const bool phatMode = (phatParam != nullptr) && (phatParam->load(std::memory_order_relaxed) > 0.5f);

    constexpr float fastReleaseMs = 50.0f;
    const float fastReleaseCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * fastReleaseMs * 0.001f));

    constexpr float ceilingLinear = 0.988553f; // -0.1 dBFS
    const float invCeiling = 1.0f / ceilingLinear;
    constexpr float minLevelLinear = 1.0e-5f; // -100 dBFS

    float* channelLeft  = ctx.mainBuffer.getWritePointer(0);
    float* channelRight = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    const float modOffset = ctx.getModulationOffset(ModTarget::CompAmount);
    float blockPeakGrDb = 0.0f;
    bool blockSaturated = false;

    // Process in sub-blocks of up to 32 samples for smooth parameter interpolation without zipper noise
    constexpr int subBlockSize = 32;
    int samplesProcessed = 0;

    while (samplesProcessed < numSamples)
    {
        const int currentBlock = juce::jmin(subBlockSize, numSamples - samplesProcessed);

        const float curAmount  = (amountParam != nullptr)  ? amountSmoothed.skip(currentBlock)  : 0.0f;
        const float curRelease = (releaseParam != nullptr) ? releaseSmoothed.skip(currentBlock) : 120.0f;
        const float curThresh  = (threshParam != nullptr)  ? threshSmoothed.skip(currentBlock)  : -18.0f;
        const float curRatio   = (ratioParam != nullptr)   ? ratioSmoothed.skip(currentBlock)   : 4.0f;
        const float curAttack  = (attackParam != nullptr)  ? attackSmoothed.skip(currentBlock)  : 5.0f;
        const float curMix     = (mixParam != nullptr)     ? mixSmoothed.skip(currentBlock)     : 1.0f;

        float effectiveAttack = curAttack;
        float effectiveRatio  = curRatio;
        float kneeWidthDb     = 4.0f;

        if (phatMode)
        {
            effectiveAttack = juce::jmin(curAttack, 0.8f);
            effectiveRatio  = juce::jmax(curRatio, 12.0f);
            kneeWidthDb     = 1.0f;
        }

        const float safeAttack = juce::jmax(0.05f, effectiveAttack);
        const float attackCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * safeAttack * 0.001f));

        const float safeRelease = juce::jmax(1.0f, curRelease);
        const float slowReleaseCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * safeRelease * 0.001f));

        const float modAmount = juce::jlimit(0.0f, 1.0f, curAmount + modOffset);
        const float effectiveThreshDb = curThresh - (36.0f * modAmount);
        const float slope = 1.0f - (1.0f / effectiveRatio);
        const float halfKnee = kneeWidthDb * 0.5f;

        // Auto-makeup formulation
        const float autoMakeupDb = (-0.5f * effectiveThreshDb * slope) + (14.0f * modAmount);
        const float autoMakeupLinear = juce::Decibels::decibelsToGain(autoMakeupDb);

        for (int i = samplesProcessed; i < samplesProcessed + currentBlock; ++i)
        {
            const float inL = channelLeft[i];
            const float inR = (channelRight != nullptr) ? channelRight[i] : inL;

            const float detectorIn = juce::jmax(std::abs(inL), std::abs(inR));
            const float inDb = juce::Decibels::gainToDecibels(std::max(detectorIn, minLevelLinear));

            // Static gain computer in dB
            float targetGrDb = 0.0f;
            if (inDb > effectiveThreshDb + halfKnee)
            {
                targetGrDb = -slope * (inDb - effectiveThreshDb);
            }
            else if (inDb > effectiveThreshDb - halfKnee)
            {
                const float diff = inDb - effectiveThreshDb + halfKnee;
                targetGrDb = -slope * (diff * diff) / (2.0f * kneeWidthDb);
            }

            // Dual-stage optical ballistics in dB
            // Stage 1: Fast recovery (~50 ms)
            if (targetGrDb < fastEnvelopeDb)
                fastEnvelopeDb = targetGrDb + attackCoeff * (fastEnvelopeDb - targetGrDb);
            else
                fastEnvelopeDb = targetGrDb + fastReleaseCoeff * (fastEnvelopeDb - targetGrDb);

            // Stage 2: Slow program-dependent recovery (user release)
            if (targetGrDb < slowEnvelopeDb)
                slowEnvelopeDb = targetGrDb + attackCoeff * (slowEnvelopeDb - targetGrDb);
            else
                slowEnvelopeDb = targetGrDb + slowReleaseCoeff * (slowEnvelopeDb - targetGrDb);

            // Composite optical gain reduction
            const float grDb = 0.5f * (fastEnvelopeDb + slowEnvelopeDb);
            blockPeakGrDb = juce::jmin(blockPeakGrDb, grDb);

            const float compGain = juce::Decibels::decibelsToGain(grDb) * autoMakeupLinear;

            float outL = inL * compGain;
            float outR = inR * compGain;

            if (phatMode)
            {
                if (std::abs(outL) > 0.93f || std::abs(outR) > 0.93f)
                    blockSaturated = true;

                // 1st-order Anti-Derivative Anti-Aliasing (ADAA) soft clipping
                outL = processADAA(outL * invCeiling, prevInNormL) * ceilingLinear;
                if (channelRight != nullptr)
                    outR = processADAA(outR * invCeiling, prevInNormR) * ceilingLinear;
            }
            else
            {
                prevInNormL = outL * invCeiling;
                if (channelRight != nullptr)
                    prevInNormR = outR * invCeiling;
            }

            // Parallel mix
            channelLeft[i] = inL * (1.0f - curMix) + outL * curMix;
            if (channelRight != nullptr)
                channelRight[i] = inR * (1.0f - curMix) + outR * curMix;
        }

        samplesProcessed += currentBlock;
    }

    gainReductionDb.store(blockPeakGrDb, std::memory_order_relaxed);
    saturationActivity.store(blockSaturated ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void CompressorDSP::randomize(juce::Random& rng)
{
    if (threshSmoothed.isSmoothing())  threshSmoothed.setCurrentAndTargetValue(rng.nextFloat() * -30.0f - 6.0f);
    if (ratioSmoothed.isSmoothing())   ratioSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 6.0f + 2.0f);
    if (attackSmoothed.isSmoothing())  attackSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 15.0f + 1.0f);
    if (releaseSmoothed.isSmoothing()) releaseSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 400.0f + 50.0f);
    if (mixSmoothed.isSmoothing())      mixSmoothed.setCurrentAndTargetValue(1.0f);
}

} // namespace wvzz::modules
