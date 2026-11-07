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
    rejDelayBufferL.fill(0.0f);
    rejDelayBufferR.fill(0.0f);
    rejDelayWriteIndex = 0;
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

    const float curBrMix = brMixSmoothed.skip(numSamples);
    lastModBrMix = juce::jlimit(0.0f, 1.0f, curBrMix + ctx.getModulationOffset(ModTarget::BPBrMix));

    const float lowCutModMult  = std::pow(2.0f, (ctx.getModulationOffset(ModTarget::BPLowCut) + ctx.getModulationOffset(ModTarget::BPBothCuts)) * 4.0f);
    const float highCutModMult = std::pow(2.0f, (ctx.getModulationOffset(ModTarget::BPHighCut) + ctx.getModulationOffset(ModTarget::BPBothCuts)) * 4.0f);
    const float lowResModOff   = ctx.getModulationOffset(ModTarget::BPLowRes);
    const float highResModOff  = ctx.getModulationOffset(ModTarget::BPHighRes);

    // Cap cutoff strictly below 0.45 * fs to prevent bilinear frequency cramping near Nyquist
    const float maxFreq = std::min(20000.0f, static_cast<float>(sampleRate * 0.45));
    const float piOverFs = juce::MathConstants<float>::pi / static_cast<float>(sampleRate);

    float* passL = ctx.mainBuffer.getWritePointer(0);
    float* rejL  = ctx.auxBandRejectBuffer.getWritePointer(0);
    float* passR = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;
    float* rejR  = (numChannels > 1) ? ctx.auxBandRejectBuffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        const float curLowCut  = lowCutSmoothed.getNextValue();
        const float curHighCut = highCutSmoothed.getNextValue();
        const float curLowRes  = lowResSmoothed.getNextValue();
        const float curHighRes = highResSmoothed.getNextValue();

        const float safeLow  = juce::jlimit(20.0f, maxFreq, curLowCut * lowCutModMult);
        const float safeHigh = juce::jlimit(safeLow, maxFreq, curHighCut * highCutModMult);

        const float modLowRes  = juce::jlimit(0.0f, 1.0f, curLowRes + lowResModOff);
        const float modHighRes = juce::jlimit(0.0f, 1.0f, curHighRes + highResModOff);

        // Nyquist Q-damping: progressively damp max Q near Nyquist to prevent high-cut instability
        const float nyqRatioLow  = safeLow / maxFreq;
        const float nyqRatioHigh = safeHigh / maxFreq;
        const float maxQLow  = juce::jmap(nyqRatioLow * nyqRatioLow, 12.0f, 2.0f);
        const float maxQHigh = juce::jmap(nyqRatioHigh * nyqRatioHigh, 12.0f, 2.0f);

        const float qLow  = juce::jmap(modLowRes, 0.7071f, maxQLow);
        const float qHigh = juce::jmap(modHighRes, 0.7071f, maxQHigh);
        const float kLow  = 1.0f / qLow;
        const float kHigh = 1.0f / qHigh;

        const float gLow  = std::tan(piOverFs * safeLow);
        const float gHigh = std::tan(piOverFs * safeHigh);

        // Left Channel
        {
            const float xL = passL[i];
            auto outLowL   = svfLowL.processSample(xL, gLow, kLow);
            auto outHighL  = svfHighL.processSample(outLowL.hp, gHigh, kHigh);
            passL[i] = outHighL.lp;
            rejL[i]  = outLowL.lp + outHighL.hp;
        }

        // Right Channel
        if (passR != nullptr && rejR != nullptr)
        {
            const float xR = passR[i];
            auto outLowR   = svfLowR.processSample(xR, gLow, kLow);
            auto outHighR  = svfHighR.processSample(outLowR.hp, gHigh, kHigh);
            passR[i] = outHighR.lp;
            rejR[i]  = outLowR.lp + outHighR.hp;
        }
    }
}

void BPFilterDSP::recombine(ProcessContext& ctx) noexcept
{
    if (ctx.hasAuxSplit && lastModBrMix > 0.001f)
    {
        const int numSamples = ctx.mainBuffer.getNumSamples();
        const int numChannels = juce::jmin(ctx.mainBuffer.getNumChannels(), ctx.auxBandRejectBuffer.getNumChannels());
        const int latency = std::clamp(ctx.distortionLatencySamples, 0, maxRejDelaySamples - 1);

        for (int i = 0; i < numSamples; ++i)
        {
            const float inRejL = ctx.auxBandRejectBuffer.getSample(0, i);
            const float inRejR = (numChannels > 1) ? ctx.auxBandRejectBuffer.getSample(1, i) : inRejL;

            rejDelayBufferL[static_cast<size_t>(rejDelayWriteIndex)] = inRejL;
            rejDelayBufferR[static_cast<size_t>(rejDelayWriteIndex)] = inRejR;

            int readIdx = rejDelayWriteIndex - latency;
            if (readIdx < 0)
                readIdx += maxRejDelaySamples;

            const float delayedRejL = rejDelayBufferL[static_cast<size_t>(readIdx)];
            const float delayedRejR = rejDelayBufferR[static_cast<size_t>(readIdx)];

            if (++rejDelayWriteIndex >= maxRejDelaySamples)
                rejDelayWriteIndex = 0;

            ctx.mainBuffer.addSample(0, i, delayedRejL * lastModBrMix);
            if (numChannels > 1)
                ctx.mainBuffer.addSample(1, i, delayedRejR * lastModBrMix);
        }
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
