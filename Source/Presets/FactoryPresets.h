#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include <array>

namespace camelphat {

struct FactoryPreset
{
    const char* name;
    // Parameter values mapped by ParameterID string
    std::vector<std::pair<juce::String, float>> values;
};

inline const std::vector<FactoryPreset>& getFactoryPresets()
{
    static const std::vector<FactoryPreset> presets = {
        // 1. Neuro Reese Mangler
        {
            "Neuro Reese Mangler",
            {
                { ParameterIDs::inputGain, 0.0f },
                { ParameterIDs::bpBypass, 0.0f },
                { ParameterIDs::bpLowCut, 60.0f },
                { ParameterIDs::bpHighCut, 8000.0f },
                { ParameterIDs::bpLowRes, 0.2f },
                { ParameterIDs::bpHighRes, 0.4f },
                { ParameterIDs::bpBrMix, 0.25f },
                { ParameterIDs::distBypass, 0.0f },
                { ParameterIDs::distMech, 0.75f },
                { ParameterIDs::distTube, 0.6f },
                { ParameterIDs::distBit, 0.15f },
                { ParameterIDs::distXcita, 0.35f },
                { ParameterIDs::mmBypass, 0.0f },
                { ParameterIDs::mmType, 1.0f }, // LP 24dB
                { ParameterIDs::mmCutoff, 1200.0f },
                { ParameterIDs::mmRes, 0.65f },
                { ParameterIDs::mmMix, 1.0f },
                { ParameterIDs::mmEnvDepth, 0.45f },
                { ParameterIDs::flgBypass, 0.0f },
                { ParameterIDs::flgRate, 0.25f },
                { ParameterIDs::flgAmount, 0.35f },
                { ParameterIDs::meqBypass, 0.0f },
                { ParameterIDs::meqAmount, 0.6f },
                { ParameterIDs::meqTune, 75.0f },
                { ParameterIDs::meqPhat, 1.0f },
                { ParameterIDs::compBypass, 0.0f },
                { ParameterIDs::compAmount, 0.7f },
                { ParameterIDs::compRelease, 100.0f },
                { ParameterIDs::compPhat, 1.0f },
                { ParameterIDs::masterMix, 1.0f },
                { ParameterIDs::masterVol, 0.0f }
            }
        },
        // 2. Phat Breakbeat Smash
        {
            "Phat Breakbeat Smash",
            {
                { ParameterIDs::bpBypass, 0.0f },
                { ParameterIDs::bpLowCut, 35.0f },
                { ParameterIDs::bpHighCut, 16000.0f },
                { ParameterIDs::bpBrMix, 0.0f },
                { ParameterIDs::distBypass, 0.0f },
                { ParameterIDs::distTube, 0.55f },
                { ParameterIDs::distMech, 0.3f },
                { ParameterIDs::distXcita, 0.4f },
                { ParameterIDs::mmBypass, 1.0f },
                { ParameterIDs::flgBypass, 1.0f },
                { ParameterIDs::meqBypass, 0.0f },
                { ParameterIDs::meqAmount, 0.5f },
                { ParameterIDs::meqTune, 85.0f },
                { ParameterIDs::meqPhat, 1.0f },
                { ParameterIDs::compBypass, 0.0f },
                { ParameterIDs::compAmount, 0.85f },
                { ParameterIDs::compRelease, 80.0f },
                { ParameterIDs::compPhat, 1.0f },
                { ParameterIDs::masterMix, 0.85f },
                { ParameterIDs::masterVol, -1.0f }
            }
        },
        // 3. Sub Weight Enhancer
        {
            "Sub Weight Enhancer",
            {
                { ParameterIDs::bpBypass, 0.0f },
                { ParameterIDs::bpLowCut, 20.0f },
                { ParameterIDs::bpHighCut, 250.0f },
                { ParameterIDs::bpLowRes, 0.1f },
                { ParameterIDs::bpHighRes, 0.2f },
                { ParameterIDs::bpBrMix, 0.6f },
                { ParameterIDs::distBypass, 0.0f },
                { ParameterIDs::distTube, 0.7f },
                { ParameterIDs::mmBypass, 0.0f },
                { ParameterIDs::mmType, 0.0f }, // LP 12dB
                { ParameterIDs::mmCutoff, 220.0f },
                { ParameterIDs::flgBypass, 1.0f },
                { ParameterIDs::meqBypass, 0.0f },
                { ParameterIDs::meqAmount, 0.8f },
                { ParameterIDs::meqTune, 65.0f },
                { ParameterIDs::meqPhat, 1.0f },
                { ParameterIDs::compBypass, 0.0f },
                { ParameterIDs::compAmount, 0.5f },
                { ParameterIDs::compPhat, 0.0f },
                { ParameterIDs::masterMix, 1.0f },
                { ParameterIDs::masterVol, 0.0f }
            }
        },
        // 4. Acid Band-Reject Sweep
        {
            "Acid Band-Reject Sweep",
            {
                { ParameterIDs::bpBypass, 0.0f },
                { ParameterIDs::bpLowCut, 350.0f },
                { ParameterIDs::bpHighCut, 2500.0f },
                { ParameterIDs::bpLowRes, 0.5f },
                { ParameterIDs::bpHighRes, 0.6f },
                { ParameterIDs::bpBrMix, 0.8f },
                { ParameterIDs::distBypass, 0.0f },
                { ParameterIDs::distMech, 0.8f },
                { ParameterIDs::mmBypass, 0.0f },
                { ParameterIDs::mmType, 3.0f }, // BP 24dB
                { ParameterIDs::mmCutoff, 1400.0f },
                { ParameterIDs::mmRes, 0.8f },
                { ParameterIDs::mmEnvDepth, 0.7f },
                { ParameterIDs::flgBypass, 1.0f },
                { ParameterIDs::meqBypass, 1.0f },
                { ParameterIDs::compBypass, 0.0f },
                { ParameterIDs::compAmount, 0.6f },
                { ParameterIDs::masterMix, 1.0f },
                { ParameterIDs::masterVol, 0.0f }
            }
        },
        // 5. Lo-Fi Bit Crunch
        {
            "Lo-Fi Bit Crunch",
            {
                { ParameterIDs::bpBypass, 0.0f },
                { ParameterIDs::bpLowCut, 200.0f },
                { ParameterIDs::bpHighCut, 7000.0f },
                { ParameterIDs::distBypass, 0.0f },
                { ParameterIDs::distBit, 0.75f },
                { ParameterIDs::distTube, 0.3f },
                { ParameterIDs::mmBypass, 0.0f },
                { ParameterIDs::mmType, 0.0f }, // LP 12dB
                { ParameterIDs::mmCutoff, 6000.0f },
                { ParameterIDs::flgBypass, 1.0f },
                { ParameterIDs::meqBypass, 1.0f },
                { ParameterIDs::compBypass, 0.0f },
                { ParameterIDs::compAmount, 0.4f },
                { ParameterIDs::masterMix, 0.9f },
                { ParameterIDs::masterVol, 0.0f }
            }
        },
        // 6. Warm Tube Bus Glue
        {
            "Warm Tube Bus Glue",
            {
                { ParameterIDs::bpBypass, 1.0f },
                { ParameterIDs::distBypass, 0.0f },
                { ParameterIDs::distTube, 0.35f },
                { ParameterIDs::distXcita, 0.2f },
                { ParameterIDs::mmBypass, 1.0f },
                { ParameterIDs::flgBypass, 1.0f },
                { ParameterIDs::meqBypass, 0.0f },
                { ParameterIDs::meqAmount, 0.25f },
                { ParameterIDs::meqTune, 80.0f },
                { ParameterIDs::meqPhat, 0.0f },
                { ParameterIDs::compBypass, 0.0f },
                { ParameterIDs::compAmount, 0.4f },
                { ParameterIDs::compRelease, 180.0f },
                { ParameterIDs::compPhat, 0.0f },
                { ParameterIDs::masterMix, 1.0f },
                { ParameterIDs::masterVol, 0.0f }
            }
        },
        // 7. Stepped Comb Resonator
        {
            "Stepped Comb Resonator",
            {
                { ParameterIDs::bpBypass, 1.0f },
                { ParameterIDs::distBypass, 0.0f },
                { ParameterIDs::distTube, 0.25f },
                { ParameterIDs::mmBypass, 0.0f },
                { ParameterIDs::mmType, 8.0f }, // Comb +
                { ParameterIDs::mmCutoff, 440.0f },
                { ParameterIDs::mmRes, 0.75f },
                { ParameterIDs::mmMix, 0.8f },
                { ParameterIDs::flgBypass, 1.0f },
                { ParameterIDs::meqBypass, 1.0f },
                { ParameterIDs::compBypass, 0.0f },
                { ParameterIDs::compAmount, 0.3f },
                { ParameterIDs::masterMix, 1.0f },
                { ParameterIDs::masterVol, 0.0f }
            }
        },
        // 8. Talking Vowel Wobble
        {
            "Talking Vowel Wobble",
            {
                { ParameterIDs::bpBypass, 1.0f },
                { ParameterIDs::distBypass, 0.0f },
                { ParameterIDs::distMech, 0.4f },
                { ParameterIDs::mmBypass, 0.0f },
                { ParameterIDs::mmType, 11.0f }, // Vowel / Formant
                { ParameterIDs::mmCutoff, 800.0f },
                { ParameterIDs::mmRes, 0.6f },
                { ParameterIDs::mmMix, 1.0f },
                { ParameterIDs::mmEnvDepth, 0.6f },
                { ParameterIDs::flgBypass, 1.0f },
                { ParameterIDs::meqBypass, 1.0f },
                { ParameterIDs::compBypass, 0.0f },
                { ParameterIDs::compAmount, 0.5f },
                { ParameterIDs::masterMix, 1.0f },
                { ParameterIDs::masterVol, 0.0f }
            }
        },
        // 9. Metallic RingMod Perc
        {
            "Metallic RingMod Perc",
            {
                { ParameterIDs::bpBypass, 1.0f },
                { ParameterIDs::distBypass, 0.0f },
                { ParameterIDs::distTube, 0.4f },
                { ParameterIDs::mmBypass, 0.0f },
                { ParameterIDs::mmType, 10.0f }, // RingMod
                { ParameterIDs::mmCutoff, 320.0f },
                { ParameterIDs::mmRes, 0.4f },
                { ParameterIDs::mmMix, 0.7f },
                { ParameterIDs::flgBypass, 1.0f },
                { ParameterIDs::meqBypass, 1.0f },
                { ParameterIDs::compBypass, 0.0f },
                { ParameterIDs::compAmount, 0.5f },
                { ParameterIDs::masterMix, 1.0f },
                { ParameterIDs::masterVol, 0.0f }
            }
        },
        // 10. Wide Quadrature Flanger
        {
            "Wide Quadrature Flanger",
            {
                { ParameterIDs::bpBypass, 1.0f },
                { ParameterIDs::distBypass, 0.0f },
                { ParameterIDs::distTube, 0.2f },
                { ParameterIDs::mmBypass, 1.0f },
                { ParameterIDs::flgBypass, 0.0f },
                { ParameterIDs::flgRate, 0.35f },
                { ParameterIDs::flgAmount, 0.65f },
                { ParameterIDs::meqBypass, 1.0f },
                { ParameterIDs::compBypass, 0.0f },
                { ParameterIDs::compAmount, 0.3f },
                { ParameterIDs::masterMix, 1.0f },
                { ParameterIDs::masterVol, 0.0f }
            }
        }
    };
    return presets;
}

} // namespace camelphat
