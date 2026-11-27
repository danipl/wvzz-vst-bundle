#include "DynDetectorDSP.h"

namespace wvzz::modules {

DynDetectorDSP::DynDetectorDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void DynDetectorDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto xoverRange = juce::NormalisableRange<float>(40.0f, 300.0f, 0.5f, 0.4f);
    auto scHpRange  = juce::NormalisableRange<float>(20.0f, 500.0f, 0.5f, 0.35f);

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Detector Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("sub_xover"), 1 }, "Sub X-Over", xoverRange, 90.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("sub_lock"), 1 }, "Sub Lock", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("sub_tight"), 1 }, "Sub Tight", 0.0f, 1.0f, 0.25f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("sc_hp"), 1 }, "Sidechain HPF", scHpRange, 80.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("sc_tilt"), 1 }, "Sidechain Tilt", -6.0f, 6.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("sc_listen"), 1 }, "SC Listen", false));
}

void DynDetectorDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam   = apvts.getRawParameterValue(paramId("bypass"));
    subXoverParam = apvts.getRawParameterValue(paramId("sub_xover"));
    subLockParam  = apvts.getRawParameterValue(paramId("sub_lock"));
    subTightParam = apvts.getRawParameterValue(paramId("sub_tight"));
    scHpParam     = apvts.getRawParameterValue(paramId("sc_hp"));
    scTiltParam   = apvts.getRawParameterValue(paramId("sc_tilt"));
    scListenParam = apvts.getRawParameterValue(paramId("sc_listen"));
}

void DynDetectorDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const double rampTimeSec = 0.015;
    subXoverSmoothed.reset(sampleRate, rampTimeSec);
    subTightSmoothed.reset(sampleRate, rampTimeSec);
    scHpSmoothed.reset(sampleRate, rampTimeSec);
    scTiltSmoothed.reset(sampleRate, rampTimeSec);

    if (subXoverParam != nullptr) subXoverSmoothed.setCurrentAndTargetValue(subXoverParam->load(std::memory_order_relaxed));
    if (subTightParam != nullptr) subTightSmoothed.setCurrentAndTargetValue(subTightParam->load(std::memory_order_relaxed));
    if (scHpParam != nullptr)     scHpSmoothed.setCurrentAndTargetValue(scHpParam->load(std::memory_order_relaxed));
    if (scTiltParam != nullptr)   scTiltSmoothed.setCurrentAndTargetValue(scTiltParam->load(std::memory_order_relaxed));

    if (subXoverParam == nullptr) subXoverSmoothed.setCurrentAndTargetValue(90.0f);
    if (subTightParam == nullptr) subTightSmoothed.setCurrentAndTargetValue(0.0f);
    if (scHpParam == nullptr)     scHpSmoothed.setCurrentAndTargetValue(30.0f);
    if (scTiltParam == nullptr)   scTiltSmoothed.setCurrentAndTargetValue(0.0f);

    reset();
}

void DynDetectorDSP::reset() noexcept
{
    subLp1L.reset(); subLp2L.reset();
    subLp1R.reset(); subLp2R.reset();
    scHpfL.reset();  scHpfR.reset();
    scTiltStateL = 0.0f;
    scTiltStateR = 0.0f;
    subEnvL = 0.0f;
    subEnvR = 0.0f;
}

void DynDetectorDSP::setParameters(float subXoverHz, bool subLock, float subTightNorm,
                                   float scHpHz, float scTiltDb, bool scListen, bool bypass) noexcept
{
    subLockFallback = subLock;
    scListenFallback = scListen;
    bypassFallback = bypass;

    subXoverSmoothed.setCurrentAndTargetValue(juce::jlimit(40.0f, 300.0f, subXoverHz));
    subTightSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, subTightNorm));
    scHpSmoothed.setCurrentAndTargetValue(juce::jlimit(20.0f, 500.0f, scHpHz));
    scTiltSmoothed.setCurrentAndTargetValue(juce::jlimit(-6.0f, 6.0f, scTiltDb));

    if (subLockParam != nullptr)
        subLockParam->store(subLock ? 1.0f : 0.0f, std::memory_order_relaxed);
    if (scListenParam != nullptr)
        scListenParam->store(scListen ? 1.0f : 0.0f, std::memory_order_relaxed);
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void DynDetectorDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) ? (bypassParam->load(std::memory_order_relaxed) > 0.5f) : bypassFallback;
    if (bypassed)
    {
        ctx.hasAuxSplit = false;
        return;
    }

    if (subXoverParam != nullptr) subXoverSmoothed.setTargetValue(subXoverParam->load(std::memory_order_relaxed));
    if (subTightParam != nullptr) subTightSmoothed.setTargetValue(subTightParam->load(std::memory_order_relaxed));
    if (scHpParam != nullptr)     scHpSmoothed.setTargetValue(scHpParam->load(std::memory_order_relaxed));
    if (scTiltParam != nullptr)   scTiltSmoothed.setTargetValue(scTiltParam->load(std::memory_order_relaxed));

    const bool subLock = (subLockParam != nullptr) ? (subLockParam->load(std::memory_order_relaxed) > 0.5f) : subLockFallback;
    ctx.hasAuxSplit = subLock;

    const float modSubXover = ctx.getModulationOffset(ModTarget::DynSubXover);
    const float modScHp     = ctx.getModulationOffset(ModTarget::DynScHp);

    float* mainL = ctx.mainBuffer.getWritePointer(0);
    float* mainR = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    float* auxL = ctx.auxBandRejectBuffer.getWritePointer(0);
    float* auxR = (ctx.auxBandRejectBuffer.getNumChannels() > 1) ? ctx.auxBandRejectBuffer.getWritePointer(1) : nullptr;

    // Sidechain buffer setup (if sidechainInputBuffer is writeable or casted to scratch)
    auto& scBuf = const_cast<juce::AudioBuffer<float>&>(ctx.sidechainInputBuffer);
    float* scL = scBuf.getWritePointer(0);
    float* scR = (scBuf.getNumChannels() > 1) ? scBuf.getWritePointer(1) : nullptr;

    constexpr int subBlockSize = 32;
    int samplesProcessed = 0;

    const float subReleaseAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.060f)); // 60 ms sub recovery
    const float subAttackAlpha  = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.005f)); // 5 ms attack

    while (samplesProcessed < numSamples)
    {
        const int currentBlock = juce::jmin(subBlockSize, numSamples - samplesProcessed);

        const float rawXover = subXoverSmoothed.skip(currentBlock);
        const float curTight = subTightSmoothed.skip(currentBlock);
        const float rawScHp  = scHpSmoothed.skip(currentBlock);
        const float curTilt  = scTiltSmoothed.skip(currentBlock);

        const float curXover = juce::jlimit(20.0f, 600.0f, rawXover * std::pow(2.0f, modSubXover * 2.0f));
        const float curScHp  = juce::jlimit(20.0f, 2000.0f, rawScHp * std::pow(2.0f, modScHp * 2.0f));

        // Setup Butterworth Q = 0.7071 for each 2-pole stage -> 4-pole Linkwitz-Riley
        constexpr float butterworthQ = 0.70710678f;
        const float subG = std::tan(juce::MathConstants<float>::pi * curXover / static_cast<float>(sampleRate));
        const float subK = 1.0f / butterworthQ;

        const float scG = std::tan(juce::MathConstants<float>::pi * curScHp / static_cast<float>(sampleRate));
        const float scK = 1.0f / butterworthQ;

        // Tilt 1-pole coeff around 1 kHz
        const float tiltAlpha = std::exp(-juce::MathConstants<float>::twoPi * 1000.0f / static_cast<float>(sampleRate));
        const float tiltGain = juce::Decibels::decibelsToGain(curTilt);

        for (int i = samplesProcessed; i < samplesProcessed + currentBlock; ++i)
        {
            const float inL = mainL[i];
            const float inR = (mainR != nullptr) ? mainR[i] : inL;

            const float rawScL = (scL != nullptr) ? scL[i] : inL;
            const float rawScR = (scR != nullptr) ? scR[i] : inR;

            // 1. Sub crossover splitting (2-stage cascaded lowpass = 24 dB/oct Linkwitz-Riley)
            const auto lp1L = subLp1L.processSample(inL, subG, subK);
            const auto lp2L = subLp2L.processSample(lp1L.lp, subG, subK);
            float subL = lp2L.lp;
            // Exact 24 dB/oct Linkwitz-Riley high-pass via 1st-stage all-pass subtraction
            const float highL = (inL - 2.0f * subK * lp1L.bp) - subL;

            const auto lp1R = subLp1R.processSample(inR, subG, subK);
            const auto lp2R = subLp2R.processSample(lp1R.lp, subG, subK);
            float subR = lp2R.lp;
            const float highR = (inR - 2.0f * subK * lp1R.bp) - subR;

            if (subLock)
            {
                // Sub Tightening clamp
                if (curTight > 0.001f)
                {
                    const float absSubL = std::abs(subL);
                    subEnvL += (absSubL > subEnvL) ? (subAttackAlpha * (absSubL - subEnvL))
                                                   : (subReleaseAlpha * (absSubL - subEnvL));
                    const float clampGainL = 1.0f / (1.0f + curTight * subEnvL * 2.0f);
                    subL *= clampGainL;

                    const float absSubR = std::abs(subR);
                    subEnvR += (absSubR > subEnvR) ? (subAttackAlpha * (absSubR - subEnvR))
                                                   : (subReleaseAlpha * (absSubR - subEnvR));
                    const float clampGainR = 1.0f / (1.0f + curTight * subEnvR * 2.0f);
                    subR *= clampGainR;
                }

                // Store isolated sub in aux buffer; pass phase-aligned upper band forward
                auxL[i] = subL;
                if (auxR != nullptr) auxR[i] = subR;

                mainL[i] = highL;
                if (mainR != nullptr) mainR[i] = highR;
            }
            else
            {
                auxL[i] = 0.0f;
                if (auxR != nullptr) auxR[i] = 0.0f;
            }

            // 2. Sidechain Detector Filter (HPF + Tilt) on sidechain input
            const auto scHpfOutL = scHpfL.processSample(rawScL, scG, scK);
            const auto scHpfOutR = scHpfR.processSample(rawScR, scG, scK);
            float detL = scHpfOutL.hp;
            float detR = scHpfOutR.hp;

            // 1-pole Tilt Shelf: low = lowpass, high = input - lowpass
            scTiltStateL = scTiltStateL * tiltAlpha + detL * (1.0f - tiltAlpha);
            detL = scTiltStateL * (1.0f / tiltGain) + (detL - scTiltStateL) * tiltGain;

            scTiltStateR = scTiltStateR * tiltAlpha + detR * (1.0f - tiltAlpha);
            detR = scTiltStateR * (1.0f / tiltGain) + (detR - scTiltStateR) * tiltGain;

            if (scL != nullptr) scL[i] = detL;
            if (scR != nullptr) scR[i] = detR;
        }

        samplesProcessed += currentBlock;
    }
}

void DynDetectorDSP::randomize(juce::Random& rng)
{
    if (subXoverSmoothed.isSmoothing()) subXoverSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 140.0f + 60.0f);
    if (subTightSmoothed.isSmoothing()) subTightSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.5f);
    if (scHpSmoothed.isSmoothing())     scHpSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 120.0f + 40.0f);
    if (scTiltSmoothed.isSmoothing())   scTiltSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 4.0f - 2.0f);
}

} // namespace wvzz::modules
