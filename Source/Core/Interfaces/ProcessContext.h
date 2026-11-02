#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace wvzz {

/**
 * @brief 21-item Modulatable Targets Enum.
 */
enum class ModTarget : int
{
    None = 0,
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
    LFO2Rate
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
