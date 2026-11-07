#include "RhythmicPumperDSP.h"

namespace wvzz::modules {

RhythmicPumperDSP::RhythmicPumperDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void RhythmicPumperDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Pumper Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("amount"), 1 }, "Pumper Amount", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("sync_div"), 1 }, "Pumper Sync Div", getPumpSyncDivNames(), 2)); // 1/4 default

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("shape"), 1 }, "Pumper Shape", getPumpShapeNames(), 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("curve"), 1 }, "Pumper Curve", 0.1f, 0.9f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("mode"), 1 }, "Pumper Mode",
        juce::StringArray{ "Tempo Sync", "Audio / Ext SC" }, 0));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("sub_only"), 1 }, "Pumper Sub Only", false));
}

void RhythmicPumperDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam  = apvts.getRawParameterValue(paramId("bypass"));
    amountParam  = apvts.getRawParameterValue(paramId("amount"));
    syncDivParam = apvts.getRawParameterValue(paramId("sync_div"));
    shapeParam   = apvts.getRawParameterValue(paramId("shape"));
    curveParam   = apvts.getRawParameterValue(paramId("curve"));
    modeParam    = apvts.getRawParameterValue(paramId("mode"));
    subOnlyParam = apvts.getRawParameterValue(paramId("sub_only"));
}

void RhythmicPumperDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const double rampTimeSec = 0.015;
    amountSmoothed.reset(sampleRate, rampTimeSec);
    curveSmoothed.reset(sampleRate, rampTimeSec);

    if (amountParam != nullptr) amountSmoothed.setCurrentAndTargetValue(amountParam->load(std::memory_order_relaxed));
    if (curveParam != nullptr)  curveSmoothed.setCurrentAndTargetValue(curveParam->load(std::memory_order_relaxed));

    reset();
}

void RhythmicPumperDSP::reset() noexcept
{
    subSvfL.reset();
    subSvfR.reset();
    freePhase = 0.0;
    prevScPeak = 0.0f;
    currentPumpGain.store(1.0f, std::memory_order_relaxed);
}

double RhythmicPumperDSP::getSyncDivBeats(int divIdx) const noexcept
{
    // "1/1", "1/2", "1/4", "1/8", "1/16", "1/4D", "1/8D", "1/4T", "1/8T"
    switch (divIdx)
    {
        case 0: return 4.0;
        case 1: return 2.0;
        case 2: return 1.0;
        case 3: return 0.5;
        case 4: return 0.25;
        case 5: return 1.5;   // 1/4D
        case 6: return 0.75;  // 1/8D
        case 7: return 2.0 / 3.0; // 1/4T
        case 8: return 1.0 / 3.0; // 1/8T
        default: return 1.0;
    }
}

float RhythmicPumperDSP::evaluatePumpCurve(float p, int shape, float curve) const noexcept
{
    // p is normalized phase in [0, 1)
    p = juce::jlimit(0.0f, 1.0f, p);

    switch (shape)
    {
        case 0: // Classic Duck: smooth cos-squared dip recovering at curve
        {
            if (p < curve)
            {
                const float normP = p / std::max(1e-3f, curve);
                return 0.5f * (1.0f - std::cos(juce::MathConstants<float>::pi * normP));
            }
            return 1.0f;
        }
        case 1: // Hard Kick Clear: instant zero for first 15% of beat, sharp ramp up
        {
            constexpr float clearEnd = 0.15f;
            if (p < clearEnd)
                return 0.0f;
            const float normP = (p - clearEnd) / (1.0f - clearEnd);
            return std::pow(normP, 1.0f / std::max(0.2f, curve * 2.0f));
        }
        case 2: // Reverse Bass: off-beat swell with peak around 0.65, cutoff before 1.0
        {
            if (p < 0.20f)
                return 0.05f; // duck kick
            if (p < 0.70f)
            {
                // Swell upwards to 1.0
                const float normP = (p - 0.20f) / 0.50f;
                return std::pow(normP, 0.75f);
            }
            // Cut off cleanly before next kick
            const float normP = (p - 0.70f) / 0.30f;
            return std::max(0.0f, 1.0f - normP * 2.5f);
        }
        case 3: // Festival Swell: exponential build-up across the bar
        {
            return std::pow(p, curve * 3.0f);
        }
        default:
            return 1.0f;
    }
}

void RhythmicPumperDSP::setParameters(float amountNorm, int syncDiv, int shape, float curveNorm,
                                      int mode, bool subOnly, bool bypass) noexcept
{
    amountSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, amountNorm));
    curveSmoothed.setCurrentAndTargetValue(juce::jlimit(0.1f, 0.9f, curveNorm));

    if (syncDivParam != nullptr)
        syncDivParam->store(static_cast<float>(juce::jlimit(0, 8, syncDiv)), std::memory_order_relaxed);
    if (shapeParam != nullptr)
        shapeParam->store(static_cast<float>(juce::jlimit(0, 3, shape)), std::memory_order_relaxed);
    if (modeParam != nullptr)
        modeParam->store(static_cast<float>(juce::jlimit(0, 1, mode)), std::memory_order_relaxed);
    if (subOnlyParam != nullptr)
        subOnlyParam->store(subOnly ? 1.0f : 0.0f, std::memory_order_relaxed);
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void RhythmicPumperDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
    {
        currentPumpGain.store(1.0f, std::memory_order_relaxed);
        return;
    }

    if (amountParam != nullptr) amountSmoothed.setTargetValue(amountParam->load(std::memory_order_relaxed));
    if (curveParam != nullptr)  curveSmoothed.setTargetValue(curveParam->load(std::memory_order_relaxed));

    const int syncDiv = (syncDivParam != nullptr) ? static_cast<int>(syncDivParam->load(std::memory_order_relaxed)) : 2;
    const int shape   = (shapeParam != nullptr)   ? static_cast<int>(shapeParam->load(std::memory_order_relaxed)) : 0;
    const int mode    = (modeParam != nullptr)    ? static_cast<int>(modeParam->load(std::memory_order_relaxed)) : 0;
    const bool subOnly= (subOnlyParam != nullptr) && (subOnlyParam->load(std::memory_order_relaxed) > 0.5f);

    const float modAmount = ctx.getModulationOffset(ModTarget::DynPumpAmount);
    const float modCurve  = ctx.getModulationOffset(ModTarget::DynPumpCurve);

    float* leftChannel  = ctx.mainBuffer.getWritePointer(0);
    float* rightChannel = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    const float* scL = (ctx.sidechainInputBuffer.getNumChannels() > 0) ? ctx.sidechainInputBuffer.getReadPointer(0) : nullptr;

    const double beatsPerCycle = getSyncDivBeats(syncDiv);
    const double safeBpm = (ctx.hostBpm > 20.0 && ctx.hostBpm < 400.0) ? ctx.hostBpm : 120.0;
    const double cycleDurationSec = beatsPerCycle * (60.0 / safeBpm);
    const double phaseIncPerSample = 1.0 / (std::max(1e-4, cycleDurationSec) * sampleRate);

    // 180 Hz Sub-only SVF parameters
    constexpr float butterworthQ = 0.70710678f;
    const float subG = std::tan(juce::MathConstants<float>::pi * 180.0f / static_cast<float>(sampleRate));
    const float subK = 1.0f / butterworthQ;

    constexpr int subBlockSize = 32;
    int samplesProcessed = 0;
    float lastGain = 1.0f;
    float minBlockGain = 1.0f;

    while (samplesProcessed < numSamples)
    {
        const int currentBlock = juce::jmin(subBlockSize, numSamples - samplesProcessed);

        const float curAmtRaw   = amountSmoothed.skip(currentBlock);
        const float curCurveRaw = curveSmoothed.skip(currentBlock);

        const float curAmount = juce::jlimit(0.0f, 1.0f, curAmtRaw + modAmount);
        const float curCurve  = juce::jlimit(0.1f, 0.9f, curCurveRaw + modCurve);

        for (int i = samplesProcessed; i < samplesProcessed + currentBlock; ++i)
        {
            float pNorm = 0.0f;

            if (mode == 0) // Tempo Sync (Host PPQ locked)
            {
                if (ctx.isHostPlaying)
                {
                    const double beatPos = ctx.hostPpqPosition + (static_cast<double>(i) / sampleRate) * (safeBpm / 60.0);
                    double phaseNorm = std::fmod(beatPos / beatsPerCycle, 1.0);
                    if (phaseNorm < 0.0) phaseNorm += 1.0;
                    pNorm = static_cast<float>(phaseNorm);
                }
                else
                {
                    freePhase += phaseIncPerSample;
                    if (freePhase >= 1.0) freePhase = std::fmod(freePhase, 1.0);
                    pNorm = static_cast<float>(freePhase);
                }
            }
            else // Audio / Ext Sidechain Trigger
            {
                const float scVal = (scL != nullptr) ? std::abs(scL[i]) : std::abs(leftChannel[i]);
                // Trigger threshold: -24 dBFS = ~0.063
                if (scVal > 0.063f && prevScPeak <= 0.063f)
                {
                    freePhase = 0.0;
                }
                prevScPeak = scVal;

                freePhase += phaseIncPerSample;
                if (freePhase > 1.0) freePhase = 1.0; // clamp at end of cycle until next trigger
                pNorm = static_cast<float>(freePhase);
            }

            const float rawCurveGain = evaluatePumpCurve(pNorm, shape, curCurve);
            const float duckGain = 1.0f - curAmount * (1.0f - rawCurveGain);
            lastGain = duckGain;
            minBlockGain = juce::jmin(minBlockGain, duckGain);

            float sL = leftChannel[i];
            float sR = (rightChannel != nullptr) ? rightChannel[i] : sL;

            if (subOnly)
            {
                // Split 180 Hz: duck only low band
                const auto outL = subSvfL.processSample(sL, subG, subK);
                const auto outR = subSvfR.processSample(sR, subG, subK);

                const float lowL = outL.lp * duckGain;
                const float lowR = outR.lp * duckGain;
                const float highL = sL - outL.lp;
                const float highR = sR - outR.lp;

                leftChannel[i] = lowL + highL;
                if (rightChannel != nullptr)
                    rightChannel[i] = lowR + highR;
            }
            else
            {
                leftChannel[i] = sL * duckGain;
                if (rightChannel != nullptr)
                    rightChannel[i] = sR * duckGain;
            }
        }

        samplesProcessed += currentBlock;
    }

    currentPumpGain.store(lastGain, std::memory_order_relaxed);
    if (minBlockGain < 1.0f)
        ctx.cumulativeGainReductionDb += juce::Decibels::gainToDecibels(std::max(0.05f, minBlockGain));
}

void RhythmicPumperDSP::randomize(juce::Random& rng)
{
    if (amountSmoothed.isSmoothing()) amountSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.5f + 0.5f);
    if (curveSmoothed.isSmoothing())  curveSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.4f + 0.3f);
}

} // namespace wvzz::modules
