#include "DistortionDSP.h"

namespace wvzz::modules {

DistortionDSP::DistortionDSP(juce::String prefix)
    : instancePrefix(std::move(prefix)),
      oversampling(2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false)
{
}

void DistortionDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Dist Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("mech"), 1 }, "Mech", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("tube"), 1 }, "Tube", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("bit"), 1 }, "Bitcrusher", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("xcita"), 1 }, "Xcita", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("mode"), 1 }, "Dist Mode",
        juce::StringArray { "Classic", "Hard Clip", "Wavefold" }, 0));
}

void DistortionDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam = apvts.getRawParameterValue(paramId("bypass"));
    mechParam   = apvts.getRawParameterValue(paramId("mech"));
    tubeParam   = apvts.getRawParameterValue(paramId("tube"));
    bitParam    = apvts.getRawParameterValue(paramId("bit"));
    xcitaParam  = apvts.getRawParameterValue(paramId("xcita"));
    modeParam   = apvts.getRawParameterValue(paramId("mode"));
}

void DistortionDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    baseSampleRate = spec.sampleRate;
    oversampling.initProcessing(spec.maximumBlockSize);

    const double osSampleRate = baseSampleRate * oversampling.getOversamplingFactor();
    juce::dsp::ProcessSpec osSpec;
    osSpec.sampleRate = osSampleRate;
    osSpec.maximumBlockSize = spec.maximumBlockSize * static_cast<juce::uint32>(oversampling.getOversamplingFactor());
    osSpec.numChannels = spec.numChannels;

    // Mech Pre-emphasis & De-emphasis filters
    mechPreFilterL.prepare(osSpec);
    mechPreFilterR.prepare(osSpec);
    mechPostFilterL.prepare(osSpec);
    mechPostFilterR.prepare(osSpec);

    auto preCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(osSampleRate, 1200.0f, 1.0f, juce::Decibels::decibelsToGain(6.0f));
    auto postCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(osSampleRate, 1200.0f, 1.0f, juce::Decibels::decibelsToGain(-6.0f));

    mechPreFilterL.coefficients = preCoeffs;
    mechPreFilterR.coefficients = preCoeffs;
    mechPostFilterL.coefficients = postCoeffs;
    mechPostFilterR.coefficients = postCoeffs;

    // Tube 1st-order 20 Hz DC blocker (avoids 2nd-order float precision collapse at 4x oversampling)
    tubeDcBlockerL.prepare(osSpec);
    tubeDcBlockerR.prepare(osSpec);
    auto dcCoeffs = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(osSampleRate, 20.0f);
    tubeDcBlockerL.coefficients = dcCoeffs;
    tubeDcBlockerR.coefficients = dcCoeffs;

    // Xcita 2.8 kHz 2-pole Butterworth pre-HPF and post-HPF (removes DC + rectified <2.8kHz intermodulation rumble)
    xcitaHpfL.prepare(osSpec);
    xcitaHpfR.prepare(osSpec);
    auto xcitaCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(osSampleRate, 2800.0f, 0.7071f);
    xcitaHpfL.coefficients = xcitaCoeffs;
    xcitaHpfR.coefficients = xcitaCoeffs;

    xcitaDcBlockerL.prepare(osSpec);
    xcitaDcBlockerR.prepare(osSpec);
    xcitaDcBlockerL.coefficients = xcitaCoeffs;
    xcitaDcBlockerR.coefficients = xcitaCoeffs;

    const double rampTimeSec = 0.015;
    mechSmoothed.reset(baseSampleRate, rampTimeSec);
    tubeSmoothed.reset(baseSampleRate, rampTimeSec);
    bitSmoothed.reset(baseSampleRate, rampTimeSec);
    xcitaSmoothed.reset(baseSampleRate, rampTimeSec);

    if (mechParam != nullptr)  mechSmoothed.setCurrentAndTargetValue(mechParam->load(std::memory_order_relaxed));
    if (tubeParam != nullptr)  tubeSmoothed.setCurrentAndTargetValue(tubeParam->load(std::memory_order_relaxed));
    if (bitParam != nullptr)   bitSmoothed.setCurrentAndTargetValue(bitParam->load(std::memory_order_relaxed));
    if (xcitaParam != nullptr) xcitaSmoothed.setCurrentAndTargetValue(xcitaParam->load(std::memory_order_relaxed));

    reset();
}

void DistortionDSP::reset() noexcept
{
    oversampling.reset();

    mechPreFilterL.reset();
    mechPreFilterR.reset();
    mechPostFilterL.reset();
    mechPostFilterR.reset();

    tubeDcBlockerL.reset();
    tubeDcBlockerR.reset();

    xcitaHpfL.reset();
    xcitaHpfR.reset();
    xcitaDcBlockerL.reset();
    xcitaDcBlockerR.reset();

    bitHoldSampleL = 0.0f;
    bitHoldSampleR = 0.0f;
    bitHoldCounterL = 0;
    bitHoldCounterR = 0;
}

void DistortionDSP::setParameters(float mechNorm, float tubeNorm, float bitNorm, float xcitaNorm, bool bypass, int mode) noexcept
{
    mechSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, mechNorm));
    tubeSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, tubeNorm));
    bitSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, bitNorm));
    xcitaSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, xcitaNorm));
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
    if (modeParam != nullptr)
        modeParam->store(static_cast<float>(juce::jlimit(0, 2, mode)), std::memory_order_relaxed);
}

void DistortionDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
    {
        ctx.distortionLatencySamples = 0;
        return;
    }

    if (mechParam != nullptr)  mechSmoothed.setTargetValue(mechParam->load(std::memory_order_relaxed));
    if (tubeParam != nullptr)  tubeSmoothed.setTargetValue(tubeParam->load(std::memory_order_relaxed));
    if (bitParam != nullptr)   bitSmoothed.setTargetValue(bitParam->load(std::memory_order_relaxed));
    if (xcitaParam != nullptr) xcitaSmoothed.setTargetValue(xcitaParam->load(std::memory_order_relaxed));

    const float curMech  = mechSmoothed.skip(numSamples);
    const float curTube  = tubeSmoothed.skip(numSamples);
    const float curBit   = bitSmoothed.skip(numSamples);
    const float curXcita = xcitaSmoothed.skip(numSamples);

    const float modMech  = juce::jlimit(0.0f, 1.0f, curMech + ctx.getModulationOffset(ModTarget::DistMech));
    const float modTube  = juce::jlimit(0.0f, 1.0f, curTube + ctx.getModulationOffset(ModTarget::DistTube));
    const float modBit   = juce::jlimit(0.0f, 1.0f, curBit + ctx.getModulationOffset(ModTarget::DistBit));
    const float modXcita = juce::jlimit(0.0f, 1.0f, curXcita + ctx.getModulationOffset(ModTarget::DistXcita));

    const bool hasMech  = (modMech > 0.001f);
    const bool hasTube  = (modTube > 0.001f);
    const bool hasBit   = (modBit > 0.001f);
    const bool hasXcita = (modXcita > 0.001f);

    if (!hasMech && !hasTube && !hasBit && !hasXcita)
    {
        ctx.distortionLatencySamples = 0;
        return;
    }

    // 1. Bitcrusher at BASE sample rate
    if (hasBit)
    {
        const float bits = juce::jmap(modBit, 16.0f, 4.0f);
        const float levels = std::pow(2.0f, bits);
        const float invLevels = 1.0f / levels;
        const int maxHold = 1 + static_cast<int>(modBit * 31.0f);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = ctx.mainBuffer.getWritePointer(ch);
            int& holdCounter = (ch == 0) ? bitHoldCounterL : bitHoldCounterR;
            float& holdSample = (ch == 0) ? bitHoldSampleL : bitHoldSampleR;

            for (int i = 0; i < numSamples; ++i)
            {
                if (++holdCounter >= maxHold)
                {
                    holdCounter = 0;
                    const float in = data[i];
                    holdSample = std::round(in * levels) * invLevels;
                }
                data[i] = (1.0f - modBit) * data[i] + modBit * holdSample;
            }
        }
    }

    if (!hasMech && !hasTube && !hasXcita)
    {
        ctx.distortionLatencySamples = 0;
        return;
    }

    ctx.distortionLatencySamples = getLatencyInSamples();

    // 2. 4x Oversampling for Mech, Tube, Xcita with Serial Cascading
    juce::dsp::AudioBlock<float> originalBlock(ctx.mainBuffer);
    auto oversampledBlock = oversampling.processSamplesUp(originalBlock);

    const int osNumSamples = static_cast<int>(oversampledBlock.getNumSamples());
    const float mechDrive = std::pow(25.0f, modMech);
    const float tubeDrive = std::pow(15.0f, modTube);
    const float xcitaDrive = 1.0f + modXcita * 9.0f;
    const int distMode = (modeParam != nullptr) ? std::clamp(static_cast<int>(modeParam->load(std::memory_order_relaxed)), 0, 2) : 0;

    const float mechMix = std::sqrt(modMech);
    const float tubeMix = std::sqrt(modTube);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* osData = oversampledBlock.getChannelPointer(ch);

        auto& preFilt  = (ch == 0) ? mechPreFilterL : mechPreFilterR;
        auto& postFilt = (ch == 0) ? mechPostFilterL : mechPostFilterR;
        auto& tubeDc   = (ch == 0) ? tubeDcBlockerL : tubeDcBlockerR;
        auto& xcitaHpf = (ch == 0) ? xcitaHpfL : xcitaHpfR;
        auto& xcitaDc  = (ch == 0) ? xcitaDcBlockerL : xcitaDcBlockerR;

        for (int i = 0; i < osNumSamples; ++i)
        {
            float x = osData[i];

            // Serial Stage 1: Mech Saturation
            if (hasMech)
            {
                const float pre = preFilt.processSample(x);
                const float dx = mechDrive * pre;
                float shaped = 0.0f;

                if (dx >= 0.0f)
                    shaped = std::tanh(dx) + 0.15f * std::sin(dx * juce::MathConstants<float>::pi);
                else
                    shaped = juce::jlimit(-0.9f, 0.0f, dx * 1.2f);

                const float mechOut = postFilt.processSample(shaped);
                x = (1.0f - mechMix) * x + mechMix * mechOut;
            }

            // Serial Stage 2: Tube Saturation
            if (hasTube)
            {
                const float x2 = x * x;
                const float rawTube = std::tanh(tubeDrive * x + 0.1f * x2) - std::tanh(0.1f * x2);
                const float tubeBlocked = tubeDc.processSample(rawTube);
                x = (1.0f - tubeMix) * x + tubeMix * tubeBlocked;
            }

            // Character waveshaping mode (Hard Clip or Wavefold)
            if (hasMech || hasTube)
            {
                if (distMode == 1) // Hard Clip
                {
                    const float clipDrive = 1.0f + 1.5f * std::max(modMech, modTube);
                    x = juce::jlimit(-0.95f, 0.95f, x * clipDrive);
                }
                else if (distMode == 2) // Wavefold
                {
                    const float foldGain = 1.0f + 3.0f * std::max(modMech, modTube);
                    const float folded = std::sin(x * foldGain);
                    const float foldMix = 0.7f * std::max(modMech, modTube);
                    x = (1.0f - foldMix) * x + foldMix * folded;
                }
            }

            // Serial Stage 3: Xcita Harmonics
            if (hasXcita)
            {
                const float xHf = xcitaHpf.processSample(x);
                const float rectified = 0.7f * xHf + 0.3f * std::abs(xHf);
                const float excited = std::tanh(xcitaDrive * rectified);
                const float xcitaBlocked = xcitaDc.processSample(excited);
                x += xcitaBlocked * (modXcita * 0.4f);
            }

            // Soft-knee safety limiter inside oversampling block
            osData[i] = std::tanh(x * 0.75f) * 1.333f;
        }
    }

    oversampling.processSamplesDown(originalBlock);
}

void DistortionDSP::randomize(juce::Random& rng)
{
    if (mechSmoothed.isSmoothing()) mechSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.8f);
    if (tubeSmoothed.isSmoothing()) tubeSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.8f);
    if (bitSmoothed.isSmoothing()) bitSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.4f);
    if (xcitaSmoothed.isSmoothing()) xcitaSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.6f);
}

} // namespace wvzz::modules
