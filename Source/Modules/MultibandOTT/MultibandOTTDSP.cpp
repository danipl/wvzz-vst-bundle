#include "MultibandOTTDSP.h"

namespace wvzz::modules {

MultibandOTTDSP::MultibandOTTDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void MultibandOTTDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "OTT Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("depth"), 1 }, "OTT Depth", 0.0f, 1.0f, 0.35f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("up_drive"), 1 }, "OTT Upward", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("down_drive"), 1 }, "OTT Downward", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("time"), 1 }, "OTT Time", 0.1f, 2.0f, 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("sheen"), 1 }, "OTT High Sheen", 0.0f, 12.0f, 0.0f));
}

void MultibandOTTDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam    = apvts.getRawParameterValue(paramId("bypass"));
    depthParam     = apvts.getRawParameterValue(paramId("depth"));
    upDriveParam   = apvts.getRawParameterValue(paramId("up_drive"));
    downDriveParam = apvts.getRawParameterValue(paramId("down_drive"));
    timeParam      = apvts.getRawParameterValue(paramId("time"));
    sheenParam     = apvts.getRawParameterValue(paramId("sheen"));
}

void MultibandOTTDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const double rampTimeSec = 0.015;
    depthSmoothed.reset(sampleRate, rampTimeSec);
    upDriveSmoothed.reset(sampleRate, rampTimeSec);
    downDriveSmoothed.reset(sampleRate, rampTimeSec);
    timeSmoothed.reset(sampleRate, rampTimeSec);
    sheenSmoothed.reset(sampleRate, rampTimeSec);

    if (depthParam != nullptr)     depthSmoothed.setCurrentAndTargetValue(depthParam->load(std::memory_order_relaxed));
    else                           depthSmoothed.setCurrentAndTargetValue(0.35f);
    if (upDriveParam != nullptr)   upDriveSmoothed.setCurrentAndTargetValue(upDriveParam->load(std::memory_order_relaxed));
    else                           upDriveSmoothed.setCurrentAndTargetValue(0.5f);
    if (downDriveParam != nullptr) downDriveSmoothed.setCurrentAndTargetValue(downDriveParam->load(std::memory_order_relaxed));
    else                           downDriveSmoothed.setCurrentAndTargetValue(0.5f);
    if (timeParam != nullptr)      timeSmoothed.setCurrentAndTargetValue(timeParam->load(std::memory_order_relaxed));
    else                           timeSmoothed.setCurrentAndTargetValue(1.0f);
    if (sheenParam != nullptr)     sheenSmoothed.setCurrentAndTargetValue(sheenParam->load(std::memory_order_relaxed));
    else                           sheenSmoothed.setCurrentAndTargetValue(0.0f);

    reset();
}

void MultibandOTTDSP::reset() noexcept
{
    xover1Lp1L.reset(); xover1Lp2L.reset();
    xover1Lp1R.reset(); xover1Lp2R.reset();
    xover2Lp1L.reset(); xover2Lp2L.reset();
    xover2Lp1R.reset(); xover2Lp2R.reset();
    sheenShelfL.reset(); sheenShelfR.reset();

    envLowL  = 0.0f; envLowR  = 0.0f;
    envMidL  = 0.0f; envMidR  = 0.0f;
    envHighL = 0.0f; envHighR = 0.0f;
}

void MultibandOTTDSP::setParameters(float depthNorm, float upDriveNorm, float downDriveNorm,
                                    float timeNorm, float sheenDb, bool bypass) noexcept
{
    depthSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, depthNorm));
    upDriveSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, upDriveNorm));
    downDriveSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, downDriveNorm));
    timeSmoothed.setCurrentAndTargetValue(juce::jlimit(0.1f, 2.0f, timeNorm));
    sheenSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 12.0f, sheenDb));

    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void MultibandOTTDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
        return;

    if (depthParam != nullptr)     depthSmoothed.setTargetValue(depthParam->load(std::memory_order_relaxed));
    if (upDriveParam != nullptr)   upDriveSmoothed.setTargetValue(upDriveParam->load(std::memory_order_relaxed));
    if (downDriveParam != nullptr) downDriveSmoothed.setTargetValue(downDriveParam->load(std::memory_order_relaxed));
    if (timeParam != nullptr)      timeSmoothed.setTargetValue(timeParam->load(std::memory_order_relaxed));
    if (sheenParam != nullptr)     sheenSmoothed.setTargetValue(sheenParam->load(std::memory_order_relaxed));

    const float modDepth   = ctx.getModulationOffset(ModTarget::DynOttDepth);
    const float modUpDrive = ctx.getModulationOffset(ModTarget::DynOttUpDrive);
    const float modSheen   = ctx.getModulationOffset(ModTarget::DynOttSheen);

    float* leftChannel  = ctx.mainBuffer.getWritePointer(0);
    float* rightChannel = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    // Crossover 1: 120 Hz
    constexpr float butterworthQ = 0.70710678f;
    const float g1 = std::tan(juce::MathConstants<float>::pi * 120.0f / static_cast<float>(sampleRate));
    const float k1 = 1.0f / butterworthQ;

    // Crossover 2: 2500 Hz
    const float g2 = std::tan(juce::MathConstants<float>::pi * 2500.0f / static_cast<float>(sampleRate));
    const float k2 = 1.0f / butterworthQ;

    constexpr int subBlockSize = 32;
    int samplesProcessed = 0;
    float blockPeakGrDb = 0.0f;

    while (samplesProcessed < numSamples)
    {
        const int currentBlock = juce::jmin(subBlockSize, numSamples - samplesProcessed);

        const float curDepthRaw = depthSmoothed.skip(currentBlock);
        const float curUpRaw    = upDriveSmoothed.skip(currentBlock);
        const float curDown     = downDriveSmoothed.skip(currentBlock);
        const float curTime     = timeSmoothed.skip(currentBlock);
        const float curSheenRaw = sheenSmoothed.skip(currentBlock);

        const float curDepth = juce::jlimit(0.0f, 1.0f, curDepthRaw + modDepth);
        const float curUp    = juce::jlimit(0.0f, 1.0f, curUpRaw + modUpDrive);
        const float curSheenDb = juce::jlimit(0.0f, 12.0f, curSheenRaw + modSheen * 12.0f);
        const float sheenLinear = juce::Decibels::decibelsToGain(curSheenDb);

        // Attack/Release ballistics scaled by curTime (true 1-pole leaky integrator alphas)
        const float attTimeSec = std::max(0.0005f, 0.005f * curTime);
        const float relTimeSec = std::max(0.005f,  0.050f * curTime);
        const float attAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * attTimeSec));
        const float relAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * relTimeSec));

        // Threshold constants
        constexpr float downThreshLinear = 0.25f;   // -12 dBFS
        constexpr float upThreshLinear   = 0.0794f; // -22 dBFS
        constexpr float floorLinear      = 0.00177f; // -55 dBFS

        for (int i = samplesProcessed; i < samplesProcessed + currentBlock; ++i)
        {
            const float inL = leftChannel[i];
            const float inR = (rightChannel != nullptr) ? rightChannel[i] : inL;

            // 1. Crossover 1 (120 Hz) -> Low and phase-aligned AboveLow (LR4)
            const auto x1L_1 = xover1Lp1L.processSample(inL, g1, k1);
            const auto x1L_2 = xover1Lp2L.processSample(x1L_1.lp, g1, k1);
            float lowL = x1L_2.lp;
            float aboveLowL = (inL - 2.0f * k1 * x1L_1.bp) - lowL;

            const auto x1R_1 = xover1Lp1R.processSample(inR, g1, k1);
            const auto x1R_2 = xover1Lp2R.processSample(x1R_1.lp, g1, k1);
            float lowR = x1R_2.lp;
            float aboveLowR = (inR - 2.0f * k1 * x1R_1.bp) - lowR;

            // Phase-align Low band with Crossover 2 (2500 Hz all-pass) so Low + Mid + High sums flat
            const auto ap2L = sheenShelfL.processSample(lowL, g2, k2);
            lowL = lowL - 2.0f * k2 * ap2L.bp;

            const auto ap2R = sheenShelfR.processSample(lowR, g2, k2);
            lowR = lowR - 2.0f * k2 * ap2R.bp;

            // 2. Crossover 2 (2500 Hz) on AboveLow -> Mid and phase-aligned High (LR4)
            const auto x2L_1 = xover2Lp1L.processSample(aboveLowL, g2, k2);
            const auto x2L_2 = xover2Lp2L.processSample(x2L_1.lp, g2, k2);
            float midL = x2L_2.lp;
            float highL = (aboveLowL - 2.0f * k2 * x2L_1.bp) - midL;

            const auto x2R_1 = xover2Lp1R.processSample(aboveLowR, g2, k2);
            const auto x2R_2 = xover2Lp2R.processSample(x2R_1.lp, g2, k2);
            float midR = x2R_2.lp;
            float highR = (aboveLowR - 2.0f * k2 * x2R_1.bp) - midR;

            // Apply High Sheen to High Band before dynamics
            highL *= sheenLinear;
            highR *= sheenLinear;

            // Reference dry all-pass sum for unity-depth phase alignment
            const float dryAlignedL = lowL + midL + (highL / sheenLinear);
            const float dryAlignedR = lowR + midR + (highR / sheenLinear);

            // Dynamics processing lambda for each band
            auto processBandDynamics = [&](float sL, float sR, float& envL, float& envR) {
                const float absL = std::abs(sL);
                const float absR = std::abs(sR);

                envL += (absL > envL) ? (attAlpha * (absL - envL)) : (relAlpha * (absL - envL));
                envR += (absR > envR) ? (attAlpha * (absR - envR)) : (relAlpha * (absR - envR));

                auto computeGain = [&](float env) -> float {
                    float g = 1.0f;
                    if (env > downThreshLinear)
                    {
                        // Downward compression: ratio = 4:1
                        const float excess = env / downThreshLinear;
                        g = 1.0f / (1.0f + curDown * (excess - 1.0f) * 0.75f);
                        const float grDb = juce::Decibels::gainToDecibels(std::max(0.05f, g)) * curDepth;
                        blockPeakGrDb = juce::jmin(blockPeakGrDb, grDb);
                    }
                    else if (env > floorLinear && env < upThreshLinear)
                    {
                        // Upward compression: pull quiet details up
                        const float deficit = upThreshLinear / std::max(1e-4f, env);
                        const float upGain = 1.0f + curUp * std::min(6.0f, deficit - 1.0f);
                        g = upGain;
                    }
                    return g;
                };

                return std::make_pair(sL * computeGain(envL), sR * computeGain(envR));
            };

            auto procLow  = processBandDynamics(lowL,  lowR,  envLowL,  envLowR);
            auto procMid  = processBandDynamics(midL,  midR,  envMidL,  envMidR);
            auto procHigh = processBandDynamics(highL, highR, envHighL, envHighR);

            const float wetL = procLow.first + procMid.first + procHigh.first;
            const float wetR = procLow.second + procMid.second + procHigh.second;

            leftChannel[i] = dryAlignedL * (1.0f - curDepth) + wetL * curDepth;
            if (rightChannel != nullptr)
                rightChannel[i] = dryAlignedR * (1.0f - curDepth) + wetR * curDepth;
        }

        samplesProcessed += currentBlock;
    }

    ctx.cumulativeGainReductionDb += blockPeakGrDb;
}

void MultibandOTTDSP::randomize(juce::Random& rng)
{
    if (depthSmoothed.isSmoothing())     depthSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.5f + 0.2f);
    if (upDriveSmoothed.isSmoothing())   upDriveSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.7f + 0.2f);
    if (downDriveSmoothed.isSmoothing()) downDriveSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.6f + 0.3f);
    if (sheenSmoothed.isSmoothing())     sheenSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 6.0f);
}

} // namespace wvzz::modules
