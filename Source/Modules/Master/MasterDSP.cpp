#include "MasterDSP.h"

namespace wvzz::modules {

MasterDSP::MasterDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void MasterDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "input_gain", 1 }, "Input Gain", -24.0f, 24.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("mix"), 1 }, "Master Mix", 0.0f, 1.0f, 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("vol"), 1 }, "Master Volume", -36.0f, 12.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Master Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("clip"), 1 }, "Master Clip", true));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("mono_bass"), 1 }, "Master Mono Bass", false));
}

void MasterDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    inputGainParam = apvts.getRawParameterValue("input_gain");
    mixParam       = apvts.getRawParameterValue(paramId("mix"));
    volParam       = apvts.getRawParameterValue(paramId("vol"));
    bypassParam    = apvts.getRawParameterValue(paramId("bypass"));
    clipParam      = apvts.getRawParameterValue(paramId("clip"));
    monoBassParam  = apvts.getRawParameterValue(paramId("mono_bass"));
}

void MasterDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    maxDelaySamples = 2048;
    dryDelayBufferL.assign(static_cast<size_t>(maxDelaySamples), 0.0f);
    dryDelayBufferR.assign(static_cast<size_t>(maxDelaySamples), 0.0f);

    const double rampTimeSec = 0.015;
    inputGainSmoothed.reset(sampleRate, rampTimeSec);
    mixSmoothed.reset(sampleRate, rampTimeSec);
    volSmoothed.reset(sampleRate, rampTimeSec);
    bypassSmoothed.reset(sampleRate, 0.010); // 10ms click-free crossfade

    if (inputGainParam != nullptr) inputGainSmoothed.setCurrentAndTargetValue(inputGainParam->load(std::memory_order_relaxed));
    if (mixParam != nullptr)       mixSmoothed.setCurrentAndTargetValue(mixParam->load(std::memory_order_relaxed));
    if (volParam != nullptr)       volSmoothed.setCurrentAndTargetValue(volParam->load(std::memory_order_relaxed));
    if (bypassParam != nullptr)    bypassSmoothed.setCurrentAndTargetValue(bypassParam->load(std::memory_order_relaxed));

    reset();
}

void MasterDSP::reset() noexcept
{
    bassMonoSvfL.reset();
    bassMonoSvfR.reset();

    peakLeft.store(0.0f, std::memory_order_relaxed);
    peakRight.store(0.0f, std::memory_order_relaxed);
    rmsLeft.store(0.0f, std::memory_order_relaxed);
    rmsRight.store(0.0f, std::memory_order_relaxed);

    inPeakLeft.store(0.0f, std::memory_order_relaxed);
    inPeakRight.store(0.0f, std::memory_order_relaxed);
    inRmsLeft.store(0.0f, std::memory_order_relaxed);
    inRmsRight.store(0.0f, std::memory_order_relaxed);

    std::fill(dryDelayBufferL.begin(), dryDelayBufferL.end(), 0.0f);
    std::fill(dryDelayBufferR.begin(), dryDelayBufferR.end(), 0.0f);
    delayWriteIndex = 0;
}

void MasterDSP::setParameters(float inputGainDb, float mixNorm, float volDb, bool bypass, bool clip, bool monoBass) noexcept
{
    inputGainSmoothed.setCurrentAndTargetValue(inputGainDb);
    mixSmoothed.setCurrentAndTargetValue(mixNorm);
    volSmoothed.setCurrentAndTargetValue(volDb);
    bypassSmoothed.setCurrentAndTargetValue(bypass ? 1.0f : 0.0f);
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
    if (clipParam != nullptr)
        clipParam->store(clip ? 1.0f : 0.0f, std::memory_order_relaxed);
    if (monoBassParam != nullptr)
        monoBassParam->store(monoBass ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void MasterDSP::applyInputGain(juce::AudioBuffer<float>& buffer) noexcept
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0)
        return;

    if (inputGainParam != nullptr)
        inputGainSmoothed.setTargetValue(inputGainParam->load(std::memory_order_relaxed));

    const float gainDb = inputGainSmoothed.skip(numSamples);
    const float linearGain = juce::Decibels::decibelsToGain(gainDb);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.applyGain(ch, 0, numSamples, linearGain);

    // Compute input metering telemetry
    float blockPeakL = 0.0f;
    float blockPeakR = 0.0f;
    float blockSumSqL = 0.0f;
    float blockSumSqR = 0.0f;

    const float* inL = buffer.getReadPointer(0);
    const float* inR = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : inL;

    for (int i = 0; i < numSamples; ++i)
    {
        const float sL = inL[i];
        const float sR = inR[i];
        blockPeakL = juce::jmax(blockPeakL, std::abs(sL));
        blockPeakR = juce::jmax(blockPeakR, std::abs(sR));
        blockSumSqL += sL * sL;
        blockSumSqR += sR * sR;
    }

    const float blockRmsL = std::sqrt(blockSumSqL / static_cast<float>(numSamples));
    const float blockRmsR = std::sqrt(blockSumSqR / static_cast<float>(numSamples));

    updateMeter(inPeakLeft, blockPeakL);
    updateMeter(inPeakRight, blockPeakR);
    updateMeter(inRmsLeft, blockRmsL);
    updateMeter(inRmsRight, blockRmsR);
}

void MasterDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels(), ctx.sidechainInputBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    if (mixParam != nullptr) mixSmoothed.setTargetValue(mixParam->load(std::memory_order_relaxed));
    if (volParam != nullptr) volSmoothed.setTargetValue(volParam->load(std::memory_order_relaxed));
    if (bypassParam != nullptr) bypassSmoothed.setTargetValue(bypassParam->load(std::memory_order_relaxed));

    const float curMix = mixSmoothed.skip(numSamples);
    const float curVol = volSmoothed.skip(numSamples);

    const float modMix = juce::jlimit(0.0f, 1.0f, curMix + ctx.getModulationOffset(ModTarget::MasterMix));
    const float modVolDb = curVol + ctx.getModulationOffset(ModTarget::MasterVol) * 24.0f;
    const float linearGain = juce::Decibels::decibelsToGain(juce::jlimit(-36.0f, 12.0f, modVolDb));

    const bool shouldClip = (clipParam == nullptr) || (clipParam->load(std::memory_order_relaxed) > 0.5f);
    const bool monoBass = (monoBassParam != nullptr) && (monoBassParam->load(std::memory_order_relaxed) > 0.5f);

    // Active oversampling latency from ProcessContext
    const int activeLatency = std::clamp(ctx.distortionLatencySamples, 0, maxDelaySamples - 1);

    // 120 Hz Cytomic SVF crossover for Sub-Bass Mono Maker (Q = 0.7071)
    const float gMono = std::tan(juce::MathConstants<float>::pi * 120.0f / static_cast<float>(sampleRate));
    constexpr float kMono = 1.41421356f;

    float blockPeakL = 0.0f;
    float blockPeakR = 0.0f;
    float blockSumSqL = 0.0f;
    float blockSumSqR = 0.0f;

    float* wetL = ctx.mainBuffer.getWritePointer(0);
    const float* dryL = ctx.sidechainInputBuffer.getReadPointer(0);

    float* wetR = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;
    const float* dryR = (numChannels > 1) ? ctx.sidechainInputBuffer.getReadPointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        const float rawDryL = dryL[i];
        const float rawDryR = (dryR != nullptr) ? dryR[i] : rawDryL;

        dryDelayBufferL[static_cast<size_t>(delayWriteIndex)] = rawDryL;
        dryDelayBufferR[static_cast<size_t>(delayWriteIndex)] = rawDryR;

        int readIdx = delayWriteIndex - activeLatency;
        if (readIdx < 0)
            readIdx += maxDelaySamples;

        const float delayedDryL = dryDelayBufferL[static_cast<size_t>(readIdx)];
        const float delayedDryR = dryDelayBufferR[static_cast<size_t>(readIdx)];

        if (++delayWriteIndex >= maxDelaySamples)
            delayWriteIndex = 0;

        const float bpWeight = bypassSmoothed.getNextValue();

        // 1. Dry/Wet Blending with Latency Compensation
        float sampleL = (1.0f - modMix) * delayedDryL + modMix * wetL[i];
        float sampleR = (wetR != nullptr) ? ((1.0f - modMix) * delayedDryR + modMix * wetR[i]) : sampleL;

        // 2. Output Gain & Modulation
        sampleL *= linearGain;
        sampleR *= linearGain;

        // 3. Sub-Bass Mono Maker (< 120 Hz)
        if (monoBass && numChannels > 1)
        {
            const float lowL = bassMonoSvfL.processSample(sampleL, gMono, kMono).lp;
            const float lowR = bassMonoSvfR.processSample(sampleR, gMono, kMono).lp;
            const float monoLow = 0.5f * (lowL + lowR);

            sampleL = (sampleL - lowL) + monoLow;
            sampleR = (sampleR - lowR) + monoLow;
        }

        // 4. Output Clipper (0.0 dBFS safety ceiling)
        if (shouldClip)
        {
            sampleL = juce::jlimit(-1.0f, 1.0f, sampleL);
            sampleR = juce::jlimit(-1.0f, 1.0f, sampleR);
        }
        else
        {
            sampleL = juce::jlimit(-8.0f, 8.0f, sampleL);
            sampleR = juce::jlimit(-8.0f, 8.0f, sampleR);
        }

        // 5. Click-free smoothed master bypass crossfade
        if (bpWeight > 0.0f)
        {
            sampleL = (1.0f - bpWeight) * sampleL + bpWeight * delayedDryL;
            sampleR = (1.0f - bpWeight) * sampleR + bpWeight * delayedDryR;
        }

        if (!std::isfinite(sampleL)) sampleL = 0.0f;
        if (!std::isfinite(sampleR)) sampleR = 0.0f;

        wetL[i] = sampleL;
        blockPeakL = juce::jmax(blockPeakL, std::abs(sampleL));
        blockSumSqL += sampleL * sampleL;

        if (wetR != nullptr)
        {
            wetR[i] = sampleR;
            blockPeakR = juce::jmax(blockPeakR, std::abs(sampleR));
            blockSumSqR += sampleR * sampleR;
        }
    }

    const float blockRmsL = std::sqrt(blockSumSqL / static_cast<float>(numSamples));
    const float blockRmsR = (wetR != nullptr) ? std::sqrt(blockSumSqR / static_cast<float>(numSamples)) : blockRmsL;

    updateMeter(peakLeft, blockPeakL);
    updateMeter(peakRight, (wetR != nullptr) ? blockPeakR : blockPeakL);
    updateMeter(rmsLeft, blockRmsL);
    updateMeter(rmsRight, blockRmsR);
}

void MasterDSP::randomize(juce::Random& rng)
{
    if (mixSmoothed.isSmoothing()) mixSmoothed.setCurrentAndTargetValue(0.4f + rng.nextFloat() * 0.6f);
    if (volSmoothed.isSmoothing()) volSmoothed.setCurrentAndTargetValue(0.0f);
}

} // namespace wvzz::modules
