#include "BPFilterDSP.h"

namespace wvzz::modules {

BPFilterDSP::BPFilterDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void BPFilterDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto freqRange = juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.25f);

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "BP Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("low_cut"), 1 }, "BP Low Cut", freqRange, 20.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("high_cut"), 1 }, "BP High Cut", freqRange, 20000.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("link"), 1 }, "BP Link", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("low_res"), 1 }, "BP Low Res", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("high_res"), 1 }, "BP High Res", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("br_mix"), 1 }, "BP BR Mix", 0.0f, 1.0f, 0.0f));
}

void BPFilterDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam  = apvts.getRawParameterValue(paramId("bypass"));
    lowCutParam  = apvts.getRawParameterValue(paramId("low_cut"));
    highCutParam = apvts.getRawParameterValue(paramId("high_cut"));
    linkParam    = apvts.getRawParameterValue(paramId("link"));
    lowResParam  = apvts.getRawParameterValue(paramId("low_res"));
    highResParam = apvts.getRawParameterValue(paramId("high_res"));
    brMixParam   = apvts.getRawParameterValue(paramId("br_mix"));
}

void BPFilterDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const double rampTimeSec = 0.015;
    lowCutSmoothed.reset(sampleRate, rampTimeSec);
    highCutSmoothed.reset(sampleRate, rampTimeSec);
    lowResSmoothed.reset(sampleRate, rampTimeSec);
    highResSmoothed.reset(sampleRate, rampTimeSec);
    brMixSmoothed.reset(sampleRate, rampTimeSec);

    if (lowCutParam != nullptr)  lowCutSmoothed.setCurrentAndTargetValue(lowCutParam->load(std::memory_order_relaxed));
    if (highCutParam != nullptr) highCutSmoothed.setCurrentAndTargetValue(highCutParam->load(std::memory_order_relaxed));
    if (lowResParam != nullptr)  lowResSmoothed.setCurrentAndTargetValue(lowResParam->load(std::memory_order_relaxed));
    if (highResParam != nullptr) highResSmoothed.setCurrentAndTargetValue(highResParam->load(std::memory_order_relaxed));
    if (brMixParam != nullptr)   brMixSmoothed.setCurrentAndTargetValue(brMixParam->load(std::memory_order_relaxed));

    reset();
}

void BPFilterDSP::reset() noexcept
{
    svfLowL.reset();
    svfLowR.reset();
    svfHighL.reset();
    svfHighR.reset();
    svfHighCutInL.reset();
    svfHighCutInR.reset();
    lastModBrMix = 0.0f;
}

void BPFilterDSP::setCutoffs(float lowCutHz, float highCutHz) noexcept
{
    lowCutSmoothed.setCurrentAndTargetValue(lowCutHz);
    highCutSmoothed.setCurrentAndTargetValue(highCutHz);
}

void BPFilterDSP::setResonance(float lowRes, float highRes) noexcept
{
    lowResSmoothed.setCurrentAndTargetValue(lowRes);
    highResSmoothed.setCurrentAndTargetValue(highRes);
}

void BPFilterDSP::setBrMix(float brMix) noexcept
{
    brMixSmoothed.setCurrentAndTargetValue(brMix);
}

void BPFilterDSP::setBypass(bool bypassed) noexcept
{
    if (bypassParam != nullptr)
        bypassParam->store(bypassed ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void BPFilterDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels(), ctx.auxBandRejectBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);

    if (bypassed)
    {
        ctx.hasAuxSplit = false;
        for (int ch = 0; ch < numChannels; ++ch)
            ctx.auxBandRejectBuffer.clear(ch, 0, numSamples);
        lastModBrMix = 0.0f;
        return;
    }

    ctx.hasAuxSplit = true;

    // Update smoother targets
    if (lowCutParam != nullptr)  lowCutSmoothed.setTargetValue(lowCutParam->load(std::memory_order_relaxed));
    if (highCutParam != nullptr) highCutSmoothed.setTargetValue(highCutParam->load(std::memory_order_relaxed));
    if (lowResParam != nullptr)  lowResSmoothed.setTargetValue(lowResParam->load(std::memory_order_relaxed));
    if (highResParam != nullptr) highResSmoothed.setTargetValue(highResParam->load(std::memory_order_relaxed));
    if (brMixParam != nullptr)   brMixSmoothed.setTargetValue(brMixParam->load(std::memory_order_relaxed));

    const float curLowCut  = lowCutSmoothed.skip(numSamples);
    const float curHighCut = highCutSmoothed.skip(numSamples);
    const float curLowRes  = lowResSmoothed.skip(numSamples);
    const float curHighRes = highResSmoothed.skip(numSamples);
    const float curBrMix   = brMixSmoothed.skip(numSamples);

    // Apply modulation offsets from ProcessContext
    const float modLowCut  = juce::jlimit(20.0f, 20000.0f, curLowCut * std::pow(2.0f, (ctx.getModulationOffset(ModTarget::BPLowCut) + ctx.getModulationOffset(ModTarget::BPBothCuts)) * 4.0f));
    const float modHighCut = juce::jlimit(modLowCut, 20000.0f, curHighCut * std::pow(2.0f, (ctx.getModulationOffset(ModTarget::BPHighCut) + ctx.getModulationOffset(ModTarget::BPBothCuts)) * 4.0f));
    const float modLowRes  = juce::jlimit(0.0f, 1.0f, curLowRes + ctx.getModulationOffset(ModTarget::BPLowRes));
    const float modHighRes = juce::jlimit(0.0f, 1.0f, curHighRes + ctx.getModulationOffset(ModTarget::BPHighRes));
    lastModBrMix = juce::jlimit(0.0f, 1.0f, curBrMix + ctx.getModulationOffset(ModTarget::BPBrMix));

    // Q mapping: linear [0.707, 12.0]
    const float qLow = juce::jmap(modLowRes, 0.707f, 12.0f);
    const float qHigh = juce::jmap(modHighRes, 0.707f, 12.0f);
    const float kLow = 1.0f / qLow;
    const float kHigh = 1.0f / qHigh;

    const float maxFreq = static_cast<float>(sampleRate * 0.49);
    const float safeLow  = juce::jlimit(20.0f, maxFreq, modLowCut);
    const float safeHigh = juce::jlimit(safeLow, maxFreq, modHighCut);

    const float gLow = std::tan(juce::MathConstants<float>::pi * safeLow / static_cast<float>(sampleRate));
    const float gHigh = std::tan(juce::MathConstants<float>::pi * safeHigh / static_cast<float>(sampleRate));

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* pass = ctx.mainBuffer.getWritePointer(ch);
        float* rej  = ctx.auxBandRejectBuffer.getWritePointer(ch);

        auto& svfL = (ch == 0) ? svfLowL : svfLowR;
        auto& svfH = (ch == 0) ? svfHighL : svfHighR;
        auto& svfHCIn = (ch == 0) ? svfHighCutInL : svfHighCutInR;

        for (int i = 0; i < numSamples; ++i)
        {
            const float x = pass[i];

            // 1. Low Cut filter on Input: extracts Low_Rejected = LP, and passband intermediate = HP
            auto outLowCut = svfL.processSample(x, gLow, kLow);
            const float lowRejected = outLowCut.lp;
            const float highPassed  = outLowCut.hp;

            // 2. High Cut filter on highPassed signal: extracts Passband = LP(highPassed)
            auto outHighCut = svfH.processSample(highPassed, gHigh, kHigh);
            const float passband = outHighCut.lp;

            // 3. High Cut filter on Input directly to extract High_Rejected = HP(Input)
            auto outHighRejected = svfHCIn.processSample(x, gHigh, kHigh);
            const float highRejected = outHighRejected.hp;

            pass[i] = passband;
            rej[i]  = lowRejected + highRejected;
        }
    }
}

void BPFilterDSP::recombine(ProcessContext& ctx) noexcept
{
    if (ctx.hasAuxSplit && lastModBrMix > 0.001f)
    {
        const int numSamples = ctx.mainBuffer.getNumSamples();
        const int numChannels = juce::jmin(ctx.mainBuffer.getNumChannels(), ctx.auxBandRejectBuffer.getNumChannels());
        for (int ch = 0; ch < numChannels; ++ch)
            ctx.mainBuffer.addFrom(ch, 0, ctx.auxBandRejectBuffer.getReadPointer(ch), numSamples, lastModBrMix);
    }
}

void BPFilterDSP::randomize(juce::Random& rng)
{
    // Perturb cutoffs and resonance musical ranges
    if (lowCutParam != nullptr)
    {
        float low = rng.nextFloat() * 400.0f + 30.0f;
        lowCutSmoothed.setCurrentAndTargetValue(low);
    }
    if (highCutParam != nullptr)
    {
        float high = rng.nextFloat() * 10000.0f + 2000.0f;
        highCutSmoothed.setCurrentAndTargetValue(high);
    }
    if (lowResParam != nullptr)
    {
        lowResSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.7f);
    }
    if (highResParam != nullptr)
    {
        highResSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.7f);
    }
}

} // namespace wvzz::modules
