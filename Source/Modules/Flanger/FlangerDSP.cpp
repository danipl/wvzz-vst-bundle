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

    // Shown with 2 decimals; the stored value keeps full precision.
    const auto twoDecimals = [](float value, int) { return juce::String(value, 2); };

    // Defaults equal the values that were fixed before these parameters existed.
    // The ranges have no step, so the defaults are stored exactly (a step would round 2.5 ms
    // to a neighbouring float and change the WvZz Distortion sound by a few 1e-5).
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("feedback"), 1 }, "Flanger Feedback",
        juce::NormalisableRange<float>(-0.95f, 0.95f), defaultFeedback,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(twoDecimals)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("depth"), 1 }, "Flanger Depth",
        juce::NormalisableRange<float>(0.0f, 5.0f), defaultDepthMs,
        juce::AudioParameterFloatAttributes().withLabel("ms").withStringFromValueFunction(twoDecimals)));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("center"), 1 }, "Flanger Center Delay",
        juce::NormalisableRange<float>(0.5f, 10.0f), defaultCenterMs,
        juce::AudioParameterFloatAttributes().withLabel("ms").withStringFromValueFunction(twoDecimals)));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("tzero"), 1 }, "Flanger Through-Zero", false));
}

void FlangerDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam = apvts.getRawParameterValue(paramId("bypass"));
    rateParam   = apvts.getRawParameterValue(paramId("rate"));
    amountParam = apvts.getRawParameterValue(paramId("amount"));
    feedbackParam = apvts.getRawParameterValue(paramId("feedback"));
    depthParam    = apvts.getRawParameterValue(paramId("depth"));
    centerParam   = apvts.getRawParameterValue(paramId("center"));
    tzeroParam    = apvts.getRawParameterValue(paramId("tzero"));
}

void FlangerDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    maxDelaySamples = static_cast<int>(sampleRate * 0.02) + 16;
    delayBufferL.assign(static_cast<size_t>(maxDelaySamples), 0.0f);
    delayBufferR.assign(static_cast<size_t>(maxDelaySamples), 0.0f);
    dryBufferL.assign(static_cast<size_t>(maxDelaySamples), 0.0f);
    dryBufferR.assign(static_cast<size_t>(maxDelaySamples), 0.0f);

    const double rampTimeSec = 0.015;
    rateSmoothed.reset(sampleRate, rampTimeSec);
    amountSmoothed.reset(sampleRate, rampTimeSec);
    feedbackSmoothed.reset(sampleRate, rampTimeSec);
    depthSmoothed.reset(sampleRate, rampTimeSec);
    centerSmoothed.reset(sampleRate, rampTimeSec);

    feedbackSmoothed.setCurrentAndTargetValue(feedbackParam != nullptr ? feedbackParam->load(std::memory_order_relaxed) : defaultFeedback);
    depthSmoothed.setCurrentAndTargetValue(depthParam != nullptr ? depthParam->load(std::memory_order_relaxed) : defaultDepthMs);
    centerSmoothed.setCurrentAndTargetValue(centerParam != nullptr ? centerParam->load(std::memory_order_relaxed) : defaultCenterMs);

    if (rateParam != nullptr)   rateSmoothed.setCurrentAndTargetValue(rateParam->load(std::memory_order_relaxed));
    if (amountParam != nullptr) amountSmoothed.setCurrentAndTargetValue(amountParam->load(std::memory_order_relaxed));

    reset();
}

void FlangerDSP::reset() noexcept
{
    std::fill(delayBufferL.begin(), delayBufferL.end(), 0.0f);
    std::fill(delayBufferR.begin(), delayBufferR.end(), 0.0f);
    std::fill(dryBufferL.begin(), dryBufferL.end(), 0.0f);
    std::fill(dryBufferR.begin(), dryBufferR.end(), 0.0f);

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
    {
        lfoTap.setActive(false);
        return;
    }

    if (rateParam != nullptr)     rateSmoothed.setTargetValue(rateParam->load(std::memory_order_relaxed));
    if (amountParam != nullptr)   amountSmoothed.setTargetValue(amountParam->load(std::memory_order_relaxed));
    if (feedbackParam != nullptr) feedbackSmoothed.setTargetValue(feedbackParam->load(std::memory_order_relaxed));
    if (depthParam != nullptr)    depthSmoothed.setTargetValue(depthParam->load(std::memory_order_relaxed));
    if (centerParam != nullptr)   centerSmoothed.setTargetValue(centerParam->load(std::memory_order_relaxed));

    const float curRate = rateSmoothed.skip(numSamples);
    const float curAmt  = amountSmoothed.skip(numSamples);
    const float feedback = juce::jlimit(-0.95f, 0.95f, feedbackSmoothed.skip(numSamples));
    const float depthMs  = juce::jlimit(0.0f, 5.0f, depthSmoothed.skip(numSamples));
    const float centerMs = juce::jlimit(0.5f, 10.0f, centerSmoothed.skip(numSamples));
    const bool throughZero = (tzeroParam != nullptr) && (tzeroParam->load(std::memory_order_relaxed) > 0.5f);

    const float modRate = juce::jlimit(0.01f, 20.0f, curRate * std::pow(2.0f, ctx.getModulationOffset(ModTarget::FlangerRate) * 3.0f));
    const float modAmt  = juce::jlimit(0.0f, 1.0f, curAmt + ctx.getModulationOffset(ModTarget::FlangerAmount));

    lfoTap.publish(static_cast<float>(lfoPhase / juce::MathConstants<double>::twoPi), 0.25f,
                   depthMs / 5.0f, dsp::LfoPhaseTap::Sine, modAmt > 0.001f);

    if (modAmt <= 0.001f)
        return;

    float* channelLeft  = ctx.mainBuffer.getWritePointer(0);
    float* channelRight = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    const double phaseIncrement = (juce::MathConstants<double>::twoPi * modRate) / sampleRate;
    const float centerDelaySamples = static_cast<float>(sampleRate * (static_cast<double>(centerMs) * 0.001));
    const float sweepModSamples    = static_cast<float>(sampleRate * (static_cast<double>(depthMs) * 0.001));
    const float maxDelay = static_cast<float>(maxDelaySamples - 4);

    for (int i = 0; i < numSamples; ++i)
    {
        const float lfoL = static_cast<float>(std::sin(lfoPhase));
        const float lfoR = static_cast<float>(std::sin(lfoPhase + juce::MathConstants<double>::halfPi));

        lfoPhase += phaseIncrement;
        if (lfoPhase >= juce::MathConstants<double>::twoPi)
            lfoPhase -= juce::MathConstants<double>::twoPi;

        // The interpolator needs at least 1 sample of delay.
        const float delayL = juce::jlimit(1.0f, maxDelay, centerDelaySamples + sweepModSamples * lfoL);
        const float delayR = juce::jlimit(1.0f, maxDelay, centerDelaySamples + sweepModSamples * lfoR);

        // Left Channel
        const float inL = channelLeft[i];
        const float delayedL = readLagrange(delayBufferL, writeIndex, delayL);
        const float writeValL = inL + std::tanh(feedback * feedbackSampleL);
        delayBufferL[static_cast<size_t>(writeIndex)] = writeValL;
        feedbackSampleL = (std::abs(delayedL) < 1.0e-15f) ? 0.0f : delayedL; // flush denormals in the feedback loop
        dryBufferL[static_cast<size_t>(writeIndex)] = inL;

        // Through-zero: the dry path is delayed by the center delay, so the wet path sweeps around it.
        const float dryL = throughZero ? readLagrange(dryBufferL, writeIndex + 1, centerDelaySamples + 1.0f) : inL;
        channelLeft[i] = (1.0f - modAmt) * dryL + modAmt * delayedL;

        // Right Channel
        if (channelRight != nullptr)
        {
            const float inR = channelRight[i];
            const float delayedR = readLagrange(delayBufferR, writeIndex, delayR);
            const float writeValR = inR + std::tanh(feedback * feedbackSampleR);
            delayBufferR[static_cast<size_t>(writeIndex)] = writeValR;
            feedbackSampleR = (std::abs(delayedR) < 1.0e-15f) ? 0.0f : delayedR;
            dryBufferR[static_cast<size_t>(writeIndex)] = inR;

            const float dryR = throughZero ? readLagrange(dryBufferR, writeIndex + 1, centerDelaySamples + 1.0f) : inR;
            channelRight[i] = (1.0f - modAmt) * dryR + modAmt * delayedR;
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
