#include "ModulationDSP.h"

namespace wvzz::modules {

ModulationDSP::ModulationDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
    rng.seed(1337);
}

void ModulationDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto lfoRateRange = juce::NormalisableRange<float>(0.02f, 30.0f, 0.01f, 0.35f);

    // LFO 1
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "lfo1_wave", 1 }, "LFO 1 Wave", getLFOWaveformNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "lfo1_sync", 1 }, "LFO 1 Sync", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lfo1_rate", 1 }, "LFO 1 Rate", lfoRateRange, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "lfo1_sync_div", 1 }, "LFO 1 Sync Div", getLFOSyncDivNames(), 4)); // 1/4 default
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "lfo1_target", 1 }, "LFO 1 Target", getModTargetNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lfo1_depth", 1 }, "LFO 1 Depth", -1.0f, 1.0f, 0.0f));

    // LFO 2
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "lfo2_wave", 1 }, "LFO 2 Wave", getLFOWaveformNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "lfo2_sync", 1 }, "LFO 2 Sync", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lfo2_rate", 1 }, "LFO 2 Rate", lfoRateRange, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "lfo2_sync_div", 1 }, "LFO 2 Sync Div", getLFOSyncDivNames(), 4));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "lfo2_target", 1 }, "LFO 2 Target", getModTargetNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "lfo2_depth", 1 }, "LFO 2 Depth", -1.0f, 1.0f, 0.0f));

    // X/Y Pad
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "xy_x", 1 }, "X/Y Pad X", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "xy_y", 1 }, "X/Y Pad Y", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "xy_x_target", 1 }, "X Target", getModTargetNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "xy_y_target", 1 }, "Y Target", getModTargetNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "xy_x_depth", 1 }, "X Depth", -1.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "xy_y_depth", 1 }, "Y Depth", -1.0f, 1.0f, 0.0f));
}

void ModulationDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    lfo1WaveParam    = apvts.getRawParameterValue("lfo1_wave");
    lfo1SyncParam    = apvts.getRawParameterValue("lfo1_sync");
    lfo1RateParam    = apvts.getRawParameterValue("lfo1_rate");
    lfo1SyncDivParam = apvts.getRawParameterValue("lfo1_sync_div");
    lfo1TargetParam  = apvts.getRawParameterValue("lfo1_target");
    lfo1DepthParam   = apvts.getRawParameterValue("lfo1_depth");

    lfo2WaveParam    = apvts.getRawParameterValue("lfo2_wave");
    lfo2SyncParam    = apvts.getRawParameterValue("lfo2_sync");
    lfo2RateParam    = apvts.getRawParameterValue("lfo2_rate");
    lfo2SyncDivParam = apvts.getRawParameterValue("lfo2_sync_div");
    lfo2TargetParam  = apvts.getRawParameterValue("lfo2_target");
    lfo2DepthParam   = apvts.getRawParameterValue("lfo2_depth");

    xyXParam       = apvts.getRawParameterValue("xy_x");
    xyYParam       = apvts.getRawParameterValue("xy_y");
    xyXTargetParam = apvts.getRawParameterValue("xy_x_target");
    xyYTargetParam = apvts.getRawParameterValue("xy_y_target");
    xyXDepthParam  = apvts.getRawParameterValue("xy_x_depth");
    xyYDepthParam  = apvts.getRawParameterValue("xy_y_depth");
}

void ModulationDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate = spec.sampleRate;
    reset();
}

void ModulationDSP::reset() noexcept
{
    lfo1Phase = 0.0;
    lfo2Phase = 0.0;
    lfo1ShCurrent = 0.0f;
    lfo1ShNext = 0.0f;
    lfo2ShCurrent = 0.0f;
    lfo2ShNext = 0.0f;
    targetOffsetsCurrent.fill(0.0f);
}

double ModulationDSP::getSyncedCycleBeats(int divIndex) const noexcept
{
    static constexpr double divBeats[8] = {
        16.0, 8.0, 4.0, 2.0, 1.0, 0.5, 0.25, 0.125
    };

    const int baseDiv = std::clamp(divIndex % 8, 0, 7);
    const int modifier = std::clamp(divIndex / 8, 0, 2); // 0: Normal, 1: Dotted, 2: Triplet

    double beats = divBeats[baseDiv];
    if (modifier == 1)      beats *= 1.5;
    else if (modifier == 2) beats *= (2.0 / 3.0);

    return std::max(1e-4, beats);
}

float ModulationDSP::getSyncedRateHz(int divIndex, double bpm) const noexcept
{
    const double safeBpm = (bpm > 20.0 && bpm < 400.0) ? bpm : 120.0;
    const double beatSec = 60.0 / safeBpm;
    const double beats = getSyncedCycleBeats(divIndex);
    const double periodSec = beats * beatSec;
    return static_cast<float>(1.0 / std::max(1e-4, periodSec));
}

float ModulationDSP::evaluateLfo(double phase, int wave, float shCurr, float shNext) noexcept
{
    const float p = static_cast<float>(phase / juce::MathConstants<double>::twoPi);
    switch (wave)
    {
        case 0: return static_cast<float>(std::sin(phase));
        case 1: return 4.0f * std::abs(p - 0.5f) - 1.0f;
        case 2: return 1.0f - 2.0f * p;
        case 3: return 2.0f * p - 1.0f;
        case 4: return (p < 0.5f) ? 1.0f : -1.0f;
        case 5: return shCurr;
        case 6:
        {
            const float t = p;
            const float t2 = t * t;
            const float t3 = t2 * t;
            const float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
            const float h01 = -2.0f * t3 + 3.0f * t2;
            return h00 * shCurr + h01 * shNext;
        }
        default: return static_cast<float>(std::sin(phase));
    }
}

void ModulationDSP::advanceLfoPhase(double& phase, float rateHz, int samples, int wave, float& shCurr, float& shNext) noexcept
{
    const double phaseInc = (juce::MathConstants<double>::twoPi * rateHz * samples) / currentSampleRate;
    phase += phaseInc;
    if (phase >= juce::MathConstants<double>::twoPi)
    {
        phase = std::fmod(phase, juce::MathConstants<double>::twoPi);
        if (wave == 5 || wave == 6)
        {
            shCurr = shNext;
            shNext = dist(rng);
        }
    }
}

void ModulationDSP::updateBlockControl(double bpm, int numSamples, double ppq, bool isPlaying) noexcept
{
    hostBpm = (bpm > 20.0 && bpm < 400.0) ? bpm : 120.0;

    // 1. Calculate LFO 1 Rate & Phase
    const int wave1      = (lfo1WaveParam != nullptr) ? static_cast<int>(lfo1WaveParam->load(std::memory_order_relaxed)) : 0;
    const bool sync1     = (lfo1SyncParam != nullptr) && (lfo1SyncParam->load(std::memory_order_relaxed) > 0.5f);
    const float freeRate1= (lfo1RateParam != nullptr) ? lfo1RateParam->load(std::memory_order_relaxed) : 1.0f;
    const int syncDiv1   = (lfo1SyncDivParam != nullptr) ? static_cast<int>(lfo1SyncDivParam->load(std::memory_order_relaxed)) : 4;
    const auto target1   = (lfo1TargetParam != nullptr) ? static_cast<ModTarget>(static_cast<int>(lfo1TargetParam->load(std::memory_order_relaxed))) : ModTarget::None;
    const float depth1   = (lfo1DepthParam != nullptr) ? lfo1DepthParam->load(std::memory_order_relaxed) : 0.0f;

    if (sync1 && isPlaying)
    {
        const double cycleBeats = getSyncedCycleBeats(syncDiv1);
        double phaseNorm = std::fmod(ppq / cycleBeats, 1.0);
        if (phaseNorm < 0.0)
            phaseNorm += 1.0;
        lfo1Phase = phaseNorm * juce::MathConstants<double>::twoPi;
    }

    const float lfo1EffectiveHz = sync1 ? getSyncedRateHz(syncDiv1, hostBpm) : freeRate1;
    const float lfo1Val = evaluateLfo(lfo1Phase, wave1, lfo1ShCurrent, lfo1ShNext);
    const float lfo1ModOutput = lfo1Val * depth1;

    advanceLfoPhase(lfo1Phase, lfo1EffectiveHz, numSamples, wave1, lfo1ShCurrent, lfo1ShNext);

    // 2. Calculate LFO 2 Rate & Phase
    const int wave2      = (lfo2WaveParam != nullptr) ? static_cast<int>(lfo2WaveParam->load(std::memory_order_relaxed)) : 0;
    const bool sync2     = (lfo2SyncParam != nullptr) && (lfo2SyncParam->load(std::memory_order_relaxed) > 0.5f);
    const float freeRate2= (lfo2RateParam != nullptr) ? lfo2RateParam->load(std::memory_order_relaxed) : 1.0f;
    const int syncDiv2   = (lfo2SyncDivParam != nullptr) ? static_cast<int>(lfo2SyncDivParam->load(std::memory_order_relaxed)) : 4;
    const auto target2   = (lfo2TargetParam != nullptr) ? static_cast<ModTarget>(static_cast<int>(lfo2TargetParam->load(std::memory_order_relaxed))) : ModTarget::None;
    const float depth2   = (lfo2DepthParam != nullptr) ? lfo2DepthParam->load(std::memory_order_relaxed) : 0.0f;

    if (sync2 && isPlaying && target1 != ModTarget::LFO2Rate)
    {
        const double cycleBeats = getSyncedCycleBeats(syncDiv2);
        double phaseNorm = std::fmod(ppq / cycleBeats, 1.0);
        if (phaseNorm < 0.0)
            phaseNorm += 1.0;
        lfo2Phase = phaseNorm * juce::MathConstants<double>::twoPi;
    }

    float lfo2BaseHz = sync2 ? getSyncedRateHz(syncDiv2, hostBpm) : freeRate2;
    if (target1 == ModTarget::LFO2Rate)
    {
        lfo2BaseHz *= std::pow(2.0f, lfo1ModOutput * 3.0f);
        lfo2BaseHz = juce::jlimit(0.01f, 100.0f, lfo2BaseHz);
    }

    const float lfo2Val = evaluateLfo(lfo2Phase, wave2, lfo2ShCurrent, lfo2ShNext);
    const float lfo2ModOutput = lfo2Val * depth2;

    advanceLfoPhase(lfo2Phase, lfo2BaseHz, numSamples, wave2, lfo2ShCurrent, lfo2ShNext);

    // 3. Evaluate X/Y Pad offsets
    const float xyXVal = (xyXParam != nullptr) ? xyXParam->load(std::memory_order_relaxed) : 0.5f;
    const float xyYVal = (xyYParam != nullptr) ? xyYParam->load(std::memory_order_relaxed) : 0.5f;
    const auto xyXTarget = (xyXTargetParam != nullptr) ? static_cast<ModTarget>(static_cast<int>(xyXTargetParam->load(std::memory_order_relaxed))) : ModTarget::None;
    const auto xyYTarget = (xyYTargetParam != nullptr) ? static_cast<ModTarget>(static_cast<int>(xyYTargetParam->load(std::memory_order_relaxed))) : ModTarget::None;
    const float xyXDepth = (xyXDepthParam != nullptr) ? xyXDepthParam->load(std::memory_order_relaxed) : 0.0f;
    const float xyYDepth = (xyYDepthParam != nullptr) ? xyYDepthParam->load(std::memory_order_relaxed) : 0.0f;

    const float xyXModOutput = (xyXVal * 2.0f - 1.0f) * xyXDepth;
    const float xyYModOutput = (xyYVal * 2.0f - 1.0f) * xyYDepth;

    // 4. Accumulate target values
    std::array<float, numTargets> newTargets;
    newTargets.fill(0.0f);

    auto applyMod = [&](ModTarget t, float val) {
        const int idx = static_cast<int>(t);
        if (idx > 0 && idx < numTargets)
            newTargets[static_cast<size_t>(idx)] += val;
    };

    applyMod(target1, lfo1ModOutput);
    applyMod(target2, lfo2ModOutput);
    applyMod(xyXTarget, xyXModOutput);
    applyMod(xyYTarget, xyYModOutput);

    targetOffsetsCurrent = newTargets;

    currentLfo1Snap.store(lfo1Val, std::memory_order_relaxed);
    currentLfo2Snap.store(lfo2Val, std::memory_order_relaxed);
    currentLfo1PhaseNorm.store(static_cast<float>(lfo1Phase / juce::MathConstants<double>::twoPi), std::memory_order_relaxed);
    currentLfo2PhaseNorm.store(static_cast<float>(lfo2Phase / juce::MathConstants<double>::twoPi), std::memory_order_relaxed);
}

void ModulationDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    if (numSamples == 0)
        return;

    updateBlockControl(ctx.hostBpm, numSamples, ctx.hostPpqPosition, ctx.isHostPlaying);

    ctx.modulationOffsets = targetOffsetsCurrent.data();
    ctx.numModulationOffsets = numTargets;
}

void ModulationDSP::randomize(juce::Random& r)
{
    juce::ignoreUnused(r);
}

} // namespace wvzz::modules
