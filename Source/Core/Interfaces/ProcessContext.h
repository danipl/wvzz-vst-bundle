#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace wvzz {

/**
 * @brief 22-item Modulatable Targets Enum.
 */
enum class ModProfile : int
{
    Distortion = 0,
    Dynamics,
    Modulation,
    Delay
};

/**
 * @brief Modulatable Targets Enum across WvZz Distortion (0..21), WvZz Dynamics (22..33),
 * WvZz Modulation (34..42), and WvZz Delay (43..52). Values are appended only, so saved sessions stay valid.
 */
enum class ModTarget : int
{
    None = 0,
    // --- Distortion Profile Targets ---
    BPLowCut,
    BPHighCut,
    BPBothCuts,
    BPLowRes,
    BPHighRes,
    BPBrMix,
    DistMech,
    DistTube,
    DistBit,
    DistXcita,
    MMCutoff,
    MMRes,
    MMMix,
    FlangerRate,
    FlangerAmount,
    MagicEQAmount,
    MagicEQTune,
    CompAmount,
    MasterMix,
    LFO2Rate,
    MasterVol,

    // --- Dynamics Profile Targets ---
    DynSubXover,
    DynScHp,
    DynTransAttack,
    DynTransSustain,
    DynOttDepth,
    DynOttUpDrive,
    DynOttSheen,
    DynCompThresh,
    DynCompMix,
    DynPumpAmount,
    DynPumpCurve,
    DynClipDrive,

    // --- Modulation Profile Targets (WvZz Modulation). Flanger reuses FlangerRate / FlangerAmount. ---
    ModChorRate,
    ModChorDepth,
    ModChorMix,
    ModPhsRate,
    ModPhsCenter,
    ModPhsFeedback,
    ModPhsMix,
    ModTremRate,
    ModTremDepth,

    // --- Delay Profile Targets (WvZz Delay) ---
    DlyTime,
    DlyFeedback,
    DlyLp,
    DlyHp,
    DlyDiffusion,
    DlyWow,
    DlyMix,
    GrainSize,
    GrainPitch,
    GrainSpray,

    // Keep last: number of ModTarget values. New targets are appended above this line.
    NumTargets
};

/**
 * @brief Pre-allocated routing and modulation context passed to IDSPModule::process().
 * Guarantees zero heap allocation, lock-free execution, and noexcept safety on the real-time audio thread.
 */
struct ProcessContext
{
    juce::AudioBuffer<float>& mainBuffer;
    const juce::AudioBuffer<float>& sidechainInputBuffer;
    juce::AudioBuffer<float>& auxBandRejectBuffer;
    bool& hasAuxSplit;

    const float* modulationOffsets = nullptr;
    int numModulationOffsets = 0;

    double hostBpm = 120.0;
    double hostPpqPosition = 0.0;
    bool isHostPlaying = false;
    int distortionLatencySamples = 0;
    float cumulativeGainReductionDb = 0.0f;

    /**
     * @brief O(1) lock-free modulation offset lookup by integer index.
     */
    [[nodiscard]] float getModulationOffset(int targetId) const noexcept
    {
        if (modulationOffsets != nullptr && targetId >= 0 && targetId < numModulationOffsets)
            return modulationOffsets[targetId];
        return 0.0f;
    }

    /**
     * @brief O(1) lock-free modulation offset lookup by ModTarget enum.
     */
    [[nodiscard]] float getModulationOffset(ModTarget target) const noexcept
    {
        return getModulationOffset(static_cast<int>(target));
    }
};

} // namespace wvzz
