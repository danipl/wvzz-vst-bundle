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
}

void DistortionDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam = apvts.getRawParameterValue(paramId("bypass"));
    mechParam   = apvts.getRawParameterValue(paramId("mech"));
    tubeParam   = apvts.getRawParameterValue(paramId("tube"));
    bitParam    = apvts.getRawParameterValue(paramId("bit"));
    xcitaParam  = apvts.getRawParameterValue(paramId("xcita"));
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

    // Tube 10 Hz DC blocker
    tubeDcBlockerL.prepare(osSpec);
    tubeDcBlockerR.prepare(osSpec);
    auto dcCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(osSampleRate, 10.0f);
    tubeDcBlockerL.coefficients = dcCoeffs;
    tubeDcBlockerR.coefficients = dcCoeffs;

    // Xcita 2.8 kHz 2-pole Butterworth HPF + 10 Hz DC blocker
    xcitaHpfL.prepare(osSpec);
    xcitaHpfR.prepare(osSpec);
    auto xcitaCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(osSampleRate, 2800.0f, 0.7071f);
    xcitaHpfL.coefficients = xcitaCoeffs;
    xcitaHpfR.coefficients = xcitaCoeffs;

    xcitaDcBlockerL.prepare(osSpec);
    xcitaDcBlockerR.prepare(osSpec);
    xcitaDcBlockerL.coefficients = dcCoeffs;
    xcitaDcBlockerR.coefficients = dcCoeffs;

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

void DistortionDSP::setParameters(float mechNorm, float tubeNorm, float bitNorm, float xcitaNorm, bool bypass) noexcept
{
    mechSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, mechNorm));
    tubeSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, tubeNorm));
    bitSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, bitNorm));
    xcitaSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, xcitaNorm));
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void DistortionDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
        return;

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
        return;

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
        return;

    // 2. 4x Oversampling for Mech, Tube, Xcita
    juce::dsp::AudioBlock<float> originalBlock(ctx.mainBuffer);
    auto oversampledBlock = oversampling.processSamplesUp(originalBlock);

    const int osNumSamples = static_cast<int>(oversampledBlock.getNumSamples());
    const float mechDrive = std::pow(25.0f, modMech);
    const float tubeDrive = std::pow(15.0f, modTube);
    const float xcitaDrive = 1.0f + modXcita * 9.0f;

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
            const float dry = osData[i];
            float out = dry;

            if (hasMech)
            {
                const float pre = preFilt.processSample(dry);
                const float dx = mechDrive * pre;
                float shaped = 0.0f;

                if (dx >= 0.0f)
                    shaped = std::tanh(dx) + 0.15f * std::sin(dx * juce::MathConstants<float>::pi);
                else
                    shaped = juce::jlimit(-0.9f, 0.0f, dx * 1.2f);

                const float mechOut = postFilt.processSample(shaped);
                out += mechOut * (modMech * 0.5f);
            }

            if (hasTube)
            {
                const float x = dry;
                const float x2 = x * x;
                const float rawTube = std::tanh(tubeDrive * x + 0.1f * x2) - std::tanh(0.1f * x2);
                const float tubeBlocked = tubeDc.processSample(rawTube);
                out += tubeBlocked * (modTube * 0.6f);
            }

            if (hasXcita)
            {
                const float xHf = xcitaHpf.processSample(dry);
                const float rectified = 0.7f * xHf + 0.3f * std::abs(xHf);
                const float excited = std::tanh(xcitaDrive * rectified);
                const float xcitaBlocked = xcitaDc.processSample(excited);
                out += xcitaBlocked * (modXcita * 0.4f);
            }

            osData[i] = out;
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
