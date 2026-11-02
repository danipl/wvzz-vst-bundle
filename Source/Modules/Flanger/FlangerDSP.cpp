#include "FlangerDSP.h"

namespace wvzz::modules {

FlangerDSP::FlangerDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void FlangerDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto rateRange = juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f, 0.35f);

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Flanger Bypass", true)); // Default 1 (bypassed)

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("rate"), 1 }, "Flanger Rate", rateRange, 0.4f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("amount"), 1 }, "Flanger Amount", 0.0f, 1.0f, 0.0f));
}

void FlangerDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam = apvts.getRawParameterValue(paramId("bypass"));
    rateParam   = apvts.getRawParameterValue(paramId("rate"));
    amountParam = apvts.getRawParameterValue(paramId("amount"));
}

void FlangerDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    maxDelaySamples = static_cast<int>(sampleRate * 0.02) + 16;
    delayBufferL.assign(static_cast<size_t>(maxDelaySamples), 0.0f);
    delayBufferR.assign(static_cast<size_t>(maxDelaySamples), 0.0f);

    const double rampTimeSec = 0.015;
    rateSmoothed.reset(sampleRate, rampTimeSec);
    amountSmoothed.reset(sampleRate, rampTimeSec);

    if (rateParam != nullptr)   rateSmoothed.setCurrentAndTargetValue(rateParam->load(std::memory_order_relaxed));
    if (amountParam != nullptr) amountSmoothed.setCurrentAndTargetValue(amountParam->load(std::memory_order_relaxed));

    reset();
}

void FlangerDSP::reset() noexcept
{
    std::fill(delayBufferL.begin(), delayBufferL.end(), 0.0f);
    std::fill(delayBufferR.begin(), delayBufferR.end(), 0.0f);

    writeIndex = 0;
    lfoPhase = 0.0;
    feedbackSampleL = 0.0f;
    feedbackSampleR = 0.0f;
}

void FlangerDSP::setParameters(float rateHz, float amountNorm, bool bypass) noexcept
{
    rateSmoothed.setCurrentAndTargetValue(juce::jlimit(0.01f, 20.0f, rateHz));
    amountSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, amountNorm));
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
}

float FlangerDSP::readLagrange(const std::vector<float>& buf, int currentWrite, float delay) const noexcept
{
    float readPos = static_cast<float>(currentWrite) - delay;
    while (readPos < 0.0f)
        readPos += static_cast<float>(maxDelaySamples);

    const int i1 = static_cast<int>(readPos);
    const int i0 = (i1 - 1 + maxDelaySamples) % maxDelaySamples;
    const int i2 = (i1 + 1) % maxDelaySamples;
    const int i3 = (i1 + 2) % maxDelaySamples;

    const float d = readPos - static_cast<float>(i1);

    const float y0 = buf[static_cast<size_t>(i0)];
    const float y1 = buf[static_cast<size_t>(i1)];
    const float y2 = buf[static_cast<size_t>(i2)];
    const float y3 = buf[static_cast<size_t>(i3)];

    const float c0 = -d * (d - 1.0f) * (d - 2.0f) / 6.0f;
    const float c1 = (d + 1.0f) * (d - 1.0f) * (d - 2.0f) / 2.0f;
    const float c2 = -(d + 1.0f) * d * (d - 2.0f) / 2.0f;
    const float c3 = (d + 1.0f) * d * (d - 1.0f) / 6.0f;

    return c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3;
}

void FlangerDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0 || maxDelaySamples <= 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
        return;

    if (rateParam != nullptr)   rateSmoothed.setTargetValue(rateParam->load(std::memory_order_relaxed));
    if (amountParam != nullptr) amountSmoothed.setTargetValue(amountParam->load(std::memory_order_relaxed));

    const float curRate = rateSmoothed.skip(numSamples);
    const float curAmt  = amountSmoothed.skip(numSamples);

    const float modRate = juce::jlimit(0.01f, 20.0f, curRate * std::pow(2.0f, ctx.getModulationOffset(ModTarget::FlangerRate) * 3.0f));
    const float modAmt  = juce::jlimit(0.0f, 1.0f, curAmt + ctx.getModulationOffset(ModTarget::FlangerAmount));

    if (modAmt <= 0.001f)
        return;

    float* channelLeft  = ctx.mainBuffer.getWritePointer(0);
    float* channelRight = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    const double phaseIncrement = (juce::MathConstants<double>::twoPi * modRate) / sampleRate;
    const float centerDelaySamples = static_cast<float>(sampleRate * 0.0025); // 2.5ms
    const float sweepModSamples    = static_cast<float>(sampleRate * 0.0020); // +/- 2.0ms
    constexpr float fixedFeedback  = 0.55f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float lfoL = static_cast<float>(std::sin(lfoPhase));
        const float lfoR = static_cast<float>(std::sin(lfoPhase + juce::MathConstants<double>::halfPi));

        lfoPhase += phaseIncrement;
        if (lfoPhase >= juce::MathConstants<double>::twoPi)
            lfoPhase -= juce::MathConstants<double>::twoPi;

        const float delayL = centerDelaySamples + sweepModSamples * lfoL;
        const float delayR = centerDelaySamples + sweepModSamples * lfoR;

        // Left Channel
        const float inL = channelLeft[i];
        const float delayedL = readLagrange(delayBufferL, writeIndex, delayL);
        const float writeValL = inL + std::tanh(fixedFeedback * feedbackSampleL);
        delayBufferL[static_cast<size_t>(writeIndex)] = writeValL;
        feedbackSampleL = delayedL;
        channelLeft[i] = (1.0f - modAmt) * inL + modAmt * delayedL;

        // Right Channel
        if (channelRight != nullptr)
        {
            const float inR = channelRight[i];
            const float delayedR = readLagrange(delayBufferR, writeIndex, delayR);
            const float writeValR = inR + std::tanh(fixedFeedback * feedbackSampleR);
            delayBufferR[static_cast<size_t>(writeIndex)] = writeValR;
            feedbackSampleR = delayedR;
            channelRight[i] = (1.0f - modAmt) * inR + modAmt * delayedR;
        }

        if (++writeIndex >= maxDelaySamples)
            writeIndex = 0;
    }
}

void FlangerDSP::randomize(juce::Random& rng)
{
    if (rateSmoothed.isSmoothing()) rateSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 2.0f + 0.1f);
    if (amountSmoothed.isSmoothing()) amountSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.6f);
}

} // namespace wvzz::modules
