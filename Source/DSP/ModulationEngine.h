#pragma once

#include "IDSPModule.h"
#include "Parameters.h"
#include <juce_core/juce_core.h>
#include <array>
#include <cmath>
#include <random>

namespace camelphat::dsp {

/**
 * @brief Modulation System (LFO 1, LFO 2, and X/Y Pad)
 *
 * - Two independent modulation oscillators computed at control rate (every 32 samples)
 * - Per-sample linear interpolation applied across target parameters
 * - 7 Waveforms:
 *     0: Sine
 *     1: Triangle
 *     2: Saw Down
 *     3: Saw Up
 *     4: Square
 *     5: Sample & Hold (White Noise stepped at Rate)
 *     6: Smooth Random (Cubic interpolated S&H)
 * - Tempo Sync: Free (0.02 to 30.0 Hz) and Host Sync (4/1 down to 1/32 including Dotted/Triplet)
 * - 21 Modulatable Targets Enum with X/Y Pad additive offsets
 */
class ModulationEngine
{
public:
    ModulationEngine()
    {
        rng.seed(1337);
    }

    void prepare(double sampleRate) noexcept
    {
        currentSampleRate = sampleRate;
        reset();
    }

    void reset() noexcept
    {
        lfo1Phase = 0.0;
        lfo2Phase = 0.0;

        lfo1ShCurrent = 0.0f;
        lfo1ShNext = 0.0f;
        lfo2ShCurrent = 0.0f;
        lfo2ShNext = 0.0f;

        sampleCounter = 0;

        targetOffsetsCurrent.fill(0.0f);
        targetOffsetsTarget.fill(0.0f);
        targetOffsetsStep.fill(0.0f);
    }

    /**
     * @brief Compute modulation values and step per-sample interpolated offsets.
     */
    void updateBlockControl(const ModulationParams& modParams,
                            double hostBpm,
                            int numSamples) noexcept
    {
        // 1. Calculate LFO 1 Rate
        const int wave1 = static_cast<int>(modParams.lfo1Wave->load(std::memory_order_relaxed));
        const bool sync1 = modParams.lfo1Sync->load(std::memory_order_relaxed) > 0.5f;
        const float freeRate1 = modParams.lfo1Rate->load(std::memory_order_relaxed);
        const int syncDiv1 = static_cast<int>(modParams.lfo1SyncDiv->load(std::memory_order_relaxed));
        const auto target1 = static_cast<ModTarget>(static_cast<int>(modParams.lfo1Target->load(std::memory_order_relaxed)));
        const float depth1 = modParams.lfo1Depth->load(std::memory_order_relaxed);

        const float lfo1EffectiveHz = sync1 ? getSyncedRateHz(syncDiv1, hostBpm) : freeRate1;

        // 2. Evaluate LFO 1 Output [-1.0, +1.0]
        const float lfo1Val = evaluateLfo(lfo1Phase, wave1, lfo1ShCurrent, lfo1ShNext);
        const float lfo1ModOutput = lfo1Val * depth1;

        // Advance LFO 1 phase (32 samples worth of phase advance)
        advanceLfoPhase(lfo1Phase, lfo1EffectiveHz, 32, wave1, lfo1ShCurrent, lfo1ShNext);

        // 3. Calculate LFO 2 Rate (with cross-modulation from LFO 1 if targeted)
        const int wave2 = static_cast<int>(modParams.lfo2Wave->load(std::memory_order_relaxed));
        const bool sync2 = modParams.lfo2Sync->load(std::memory_order_relaxed) > 0.5f;
        const float freeRate2 = modParams.lfo2Rate->load(std::memory_order_relaxed);
        const int syncDiv2 = static_cast<int>(modParams.lfo2SyncDiv->load(std::memory_order_relaxed));
        const auto target2 = static_cast<ModTarget>(static_cast<int>(modParams.lfo2Target->load(std::memory_order_relaxed)));
        const float depth2 = modParams.lfo2Depth->load(std::memory_order_relaxed);

        float lfo2BaseHz = sync2 ? getSyncedRateHz(syncDiv2, hostBpm) : freeRate2;
        if (target1 == ModTarget::LFO2Rate)
        {
            // +/- 3 octaves shift on rate
            lfo2BaseHz *= std::pow(2.0f, lfo1ModOutput * 3.0f);
            lfo2BaseHz = juce::jlimit(0.01f, 100.0f, lfo2BaseHz);
        }

        const float lfo2Val = evaluateLfo(lfo2Phase, wave2, lfo2ShCurrent, lfo2ShNext);
        const float lfo2ModOutput = lfo2Val * depth2;

        advanceLfoPhase(lfo2Phase, lfo2BaseHz, 32, wave2, lfo2ShCurrent, lfo2ShNext);

        // 4. Evaluate X/Y Controller Pad offsets
        const float xyXVal = modParams.xyX->load(std::memory_order_relaxed);
        const float xyYVal = modParams.xyY->load(std::memory_order_relaxed);
        const auto xyXTarget = static_cast<ModTarget>(static_cast<int>(modParams.xyXTarget->load(std::memory_order_relaxed)));
        const auto xyYTarget = static_cast<ModTarget>(static_cast<int>(modParams.xyYTarget->load(std::memory_order_relaxed)));
        const float xyXDepth = modParams.xyXDepth->load(std::memory_order_relaxed);
        const float xyYDepth = modParams.xyYDepth->load(std::memory_order_relaxed);

        const float xyXModOutput = (xyXVal * 2.0f - 1.0f) * xyXDepth;
        const float xyYModOutput = (xyYVal * 2.0f - 1.0f) * xyYDepth;

        // 5. Accumulate new target values across all 21 targets
        std::array<float, numTargets> newTargets;
        newTargets.fill(0.0f);

        auto applyMod = [&](ModTarget t, float val) {
            const int idx = static_cast<int>(t);
            if (idx > 0 && idx < numTargets)
            {
                newTargets[static_cast<size_t>(idx)] += val;
            }
        };

        applyMod(target1, lfo1ModOutput);
        applyMod(target2, lfo2ModOutput);
        applyMod(xyXTarget, xyXModOutput);
        applyMod(xyYTarget, xyYModOutput);

        // Update active target offsets for the control block
        targetOffsetsCurrent = newTargets;
        targetOffsetsTarget = newTargets;
        targetOffsetsStep.fill(0.0f);

        currentLfo1Snap.store(lfo1Val, std::memory_order_relaxed);
        currentLfo2Snap.store(lfo2Val, std::memory_order_relaxed);
        currentLfo1PhaseNorm.store(static_cast<float>(lfo1Phase / juce::MathConstants<double>::twoPi), std::memory_order_relaxed);
        currentLfo2PhaseNorm.store(static_cast<float>(lfo2Phase / juce::MathConstants<double>::twoPi), std::memory_order_relaxed);
    }

    /**
     * @brief Advance sample step for linear interpolation.
     */
    void stepInterpolation() noexcept
    {
        for (size_t i = 0; i < numTargets; ++i)
        {
            targetOffsetsCurrent[i] += targetOffsetsStep[i];
        }
    }

    [[nodiscard]] inline float getOffset(ModTarget target) const noexcept
    {
        const int idx = static_cast<int>(target);
        if (idx <= 0 || idx >= numTargets)
            return 0.0f;
        return targetOffsetsCurrent[static_cast<size_t>(idx)];
    }

    [[nodiscard]] float getLfo1Snapshot() const noexcept { return currentLfo1Snap.load(std::memory_order_relaxed); }
    [[nodiscard]] float getLfo2Snapshot() const noexcept { return currentLfo2Snap.load(std::memory_order_relaxed); }
    [[nodiscard]] float getLfo1Phase() const noexcept { return currentLfo1PhaseNorm.load(std::memory_order_relaxed); }
    [[nodiscard]] float getLfo2Phase() const noexcept { return currentLfo2PhaseNorm.load(std::memory_order_relaxed); }

private:
    static constexpr int numTargets = 21;

    [[nodiscard]] float getSyncedRateHz(int divIndex, double bpm) const noexcept
    {
        const double safeBpm = (bpm > 20.0 && bpm < 400.0) ? bpm : 120.0;
        const double beatSec = 60.0 / safeBpm;

        // Base divisions in quarter notes
        static constexpr double divBeats[8] = {
            16.0, // 4/1
            8.0,  // 2/1
            4.0,  // 1/1
            2.0,  // 1/2
            1.0,  // 1/4
            0.5,  // 1/8
            0.25, // 1/16
            0.125 // 1/32
        };

        const int baseDiv = divIndex % 8;
        const int modifier = divIndex / 8; // 0: Normal, 1: Dotted (1.5x period), 2: Triplet (2/3x period)

        double beats = divBeats[baseDiv];
        if (modifier == 1)
            beats *= 1.5;
        else if (modifier == 2)
            beats *= (2.0 / 3.0);

        const double periodSec = beats * beatSec;
        return static_cast<float>(1.0 / std::max(1e-4, periodSec));
    }

    [[nodiscard]] float evaluateLfo(double phase, int wave, float shCurr, float shNext) noexcept
    {
        const float p = static_cast<float>(phase / juce::MathConstants<double>::twoPi);
        switch (wave)
        {
            case 0: // Sine
                return static_cast<float>(std::sin(phase));
            case 1: // Triangle
                return 4.0f * std::abs(p - 0.5f) - 1.0f;
            case 2: // Saw Down
                return 1.0f - 2.0f * p;
            case 3: // Saw Up
                return 2.0f * p - 1.0f;
            case 4: // Square
                return (p < 0.5f) ? 1.0f : -1.0f;
            case 5: // Sample & Hold (stepped white noise)
                return shCurr;
            case 6: // Smooth Random (cubic Hermite interpolation between noise points)
            {
                const float t = p;
                const float t2 = t * t;
                const float t3 = t2 * t;
                const float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
                const float h01 = -2.0f * t3 + 3.0f * t2;
                return h00 * shCurr + h01 * shNext;
            }
            default:
                return static_cast<float>(std::sin(phase));
        }
    }

    void advanceLfoPhase(double& phase, float rateHz, int samples, int wave, float& shCurr, float& shNext) noexcept
    {
        const double phaseInc = (juce::MathConstants<double>::twoPi * rateHz * samples) / currentSampleRate;
        phase += phaseInc;
        if (phase >= juce::MathConstants<double>::twoPi)
        {
            phase = std::fmod(phase, juce::MathConstants<double>::twoPi);

            // New random step
            if (wave == 5 || wave == 6)
            {
                shCurr = shNext;
                shNext = dist(rng);
            }
        }
    }

    double currentSampleRate { 44100.0 };

    double lfo1Phase { 0.0 };
    double lfo2Phase { 0.0 };

    float lfo1ShCurrent { 0.0f };
    float lfo1ShNext { 0.0f };
    float lfo2ShCurrent { 0.0f };
    float lfo2ShNext { 0.0f };

    int sampleCounter { 0 };

    std::array<float, numTargets> targetOffsetsCurrent;
    std::array<float, numTargets> targetOffsetsTarget;
    std::array<float, numTargets> targetOffsetsStep;

    std::mt19937 rng;
    std::uniform_real_distribution<float> dist { -1.0f, 1.0f };

    std::atomic<float> currentLfo1Snap { 0.0f };
    std::atomic<float> currentLfo2Snap { 0.0f };
    std::atomic<float> currentLfo1PhaseNorm { 0.0f };
    std::atomic<float> currentLfo2PhaseNorm { 0.0f };
};

} // namespace camelphat::dsp
