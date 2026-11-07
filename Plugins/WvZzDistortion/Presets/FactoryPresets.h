#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <utility>

namespace wvzz {

struct FactoryPreset
{
    const char* name;
    // Parameter values mapped by canonical ParameterID string
    std::vector<std::pair<juce::String, float>> values;
};

inline const std::vector<FactoryPreset>& getFactoryPresets()
{
    static const std::vector<FactoryPreset> presets = {
        // 1. Neuro Reese Mangler
        {
            "Neuro Reese Mangler",
            {
                { "input_gain", 0.0f },
                { "bp_bypass", 0.0f },
                { "bp_low_cut", 60.0f },
                { "bp_high_cut", 8000.0f },
                { "bp_low_res", 0.2f },
                { "bp_high_res", 0.4f },
                { "bp_br_mix", 0.25f },
                { "dist_bypass", 0.0f },
                { "dist_mech", 0.75f },
                { "dist_tube", 0.6f },
                { "dist_bit", 0.15f },
                { "dist_xcita", 0.35f },
                { "mm_bypass", 0.0f },
                { "mm_type", 1.0f }, // LP 24dB
                { "mm_cutoff", 1200.0f },
                { "mm_res", 0.65f },
                { "mm_mix", 1.0f },
                { "mm_env_depth", 0.45f },
                { "flg_bypass", 0.0f },
                { "flg_rate", 0.25f },
                { "flg_amount", 0.35f },
                { "meq_bypass", 0.0f },
                { "meq_amount", 0.6f },
                { "meq_tune", 75.0f },
                { "meq_phat", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.7f },
                { "comp_release", 100.0f },
                { "comp_phat", 1.0f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 2. Phat Breakbeat Smash
        {
            "Phat Breakbeat Smash",
            {
                { "bp_bypass", 0.0f },
                { "bp_low_cut", 35.0f },
                { "bp_high_cut", 16000.0f },
                { "bp_br_mix", 0.0f },
                { "dist_bypass", 0.0f },
                { "dist_tube", 0.55f },
                { "dist_mech", 0.3f },
                { "dist_xcita", 0.4f },
                { "mm_bypass", 1.0f },
                { "flg_bypass", 1.0f },
                { "meq_bypass", 0.0f },
                { "meq_amount", 0.5f },
                { "meq_tune", 85.0f },
                { "meq_phat", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.85f },
                { "comp_release", 80.0f },
                { "comp_phat", 1.0f },
                { "master_mix", 0.85f },
                { "master_vol", -1.0f }
            }
        },
        // 3. Sub Weight Enhancer
        {
            "Sub Weight Enhancer",
            {
                { "bp_bypass", 0.0f },
                { "bp_low_cut", 20.0f },
                { "bp_high_cut", 250.0f },
                { "bp_low_res", 0.1f },
                { "bp_high_res", 0.2f },
                { "bp_br_mix", 0.6f },
                { "dist_bypass", 0.0f },
                { "dist_tube", 0.7f },
                { "mm_bypass", 0.0f },
                { "mm_type", 0.0f }, // LP 12dB
                { "mm_cutoff", 220.0f },
                { "flg_bypass", 1.0f },
                { "meq_bypass", 0.0f },
                { "meq_amount", 0.8f },
                { "meq_tune", 65.0f },
                { "meq_phat", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.5f },
                { "comp_phat", 0.0f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 4. Acid Band-Reject Sweep
        {
            "Acid Band-Reject Sweep",
            {
                { "bp_bypass", 0.0f },
                { "bp_low_cut", 350.0f },
                { "bp_high_cut", 2500.0f },
                { "bp_low_res", 0.5f },
                { "bp_high_res", 0.6f },
                { "bp_br_mix", 0.8f },
                { "dist_bypass", 0.0f },
                { "dist_mech", 0.8f },
                { "mm_bypass", 0.0f },
                { "mm_type", 3.0f }, // BP 24dB
                { "mm_cutoff", 1400.0f },
                { "mm_res", 0.8f },
                { "mm_env_depth", 0.7f },
                { "flg_bypass", 1.0f },
                { "meq_bypass", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.6f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 5. Lo-Fi Bit Crunch
        {
            "Lo-Fi Bit Crunch",
            {
                { "bp_bypass", 0.0f },
                { "bp_low_cut", 200.0f },
                { "bp_high_cut", 7000.0f },
                { "dist_bypass", 0.0f },
                { "dist_bit", 0.75f },
                { "dist_tube", 0.3f },
                { "mm_bypass", 0.0f },
                { "mm_type", 0.0f }, // LP 12dB
                { "mm_cutoff", 6000.0f },
                { "flg_bypass", 1.0f },
                { "meq_bypass", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.4f },
                { "master_mix", 0.9f },
                { "master_vol", 0.0f }
            }
        },
        // 6. Warm Tube Bus Glue
        {
            "Warm Tube Bus Glue",
            {
                { "bp_bypass", 1.0f },
                { "dist_bypass", 0.0f },
                { "dist_tube", 0.35f },
                { "dist_xcita", 0.2f },
                { "mm_bypass", 1.0f },
                { "flg_bypass", 1.0f },
                { "meq_bypass", 0.0f },
                { "meq_amount", 0.25f },
                { "meq_tune", 80.0f },
                { "meq_phat", 0.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.4f },
                { "comp_release", 180.0f },
                { "comp_phat", 0.0f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 7. Stepped Comb Resonator
        {
            "Stepped Comb Resonator",
            {
                { "bp_bypass", 1.0f },
                { "dist_bypass", 0.0f },
                { "dist_tube", 0.25f },
                { "mm_bypass", 0.0f },
                { "mm_type", 8.0f }, // Comb +
                { "mm_cutoff", 440.0f },
                { "mm_res", 0.75f },
                { "mm_mix", 0.8f },
                { "flg_bypass", 1.0f },
                { "meq_bypass", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.3f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 8. Talking Vowel Wobble
        {
            "Talking Vowel Wobble",
            {
                { "bp_bypass", 1.0f },
                { "dist_bypass", 0.0f },
                { "dist_mech", 0.4f },
                { "mm_bypass", 0.0f },
                { "mm_type", 11.0f }, // Vowel / Formant
                { "mm_cutoff", 800.0f },
                { "mm_res", 0.6f },
                { "mm_mix", 1.0f },
                { "mm_env_depth", 0.6f },
                { "flg_bypass", 1.0f },
                { "meq_bypass", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.5f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 9. Metallic RingMod Perc
        {
            "Metallic RingMod Perc",
            {
                { "bp_bypass", 1.0f },
                { "dist_bypass", 0.0f },
                { "dist_tube", 0.4f },
                { "mm_bypass", 0.0f },
                { "mm_type", 10.0f }, // RingMod
                { "mm_cutoff", 320.0f },
                { "mm_res", 0.4f },
                { "mm_mix", 0.7f },
                { "flg_bypass", 1.0f },
                { "meq_bypass", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.5f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 10. Wide Quadrature Flanger
        {
            "Wide Quadrature Flanger",
            {
                { "bp_bypass", 1.0f },
                { "dist_bypass", 0.0f },
                { "dist_tube", 0.2f },
                { "mm_bypass", 1.0f },
                { "flg_bypass", 0.0f },
                { "flg_rate", 0.35f },
                { "flg_amount", 0.65f },
                { "meq_bypass", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.3f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 11. Hardstyle Kick Tok & Crunch
        {
            "Hardstyle Kick Tok & Crunch",
            {
                { "bp_bypass", 0.0f },
                { "bp_low_cut", 45.0f },
                { "bp_high_cut", 6500.0f },
                { "bp_low_res", 0.2f },
                { "bp_high_res", 0.3f },
                { "bp_br_mix", 0.35f },
                { "meq_bypass", 0.0f },
                { "meq_pre", 1.0f },
                { "meq_tune", 90.0f },
                { "meq_amount", 0.85f },
                { "meq_phat", 1.0f },
                { "dist_bypass", 0.0f },
                { "dist_mode", 1.0f }, // Hard Clip
                { "dist_mech", 0.8f },
                { "dist_tube", 0.7f },
                { "dist_xcita", 0.3f },
                { "mm_bypass", 0.0f },
                { "mm_type", 1.0f }, // LP 24dB
                { "mm_cutoff", 3500.0f },
                { "mm_res", 0.4f },
                { "flg_bypass", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.75f },
                { "comp_phat", 1.0f },
                { "master_clip", 1.0f },
                { "master_mono_bass", 1.0f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 12. Euphoric Supersaw Excite
        {
            "Euphoric Supersaw Excite",
            {
                { "bp_bypass", 0.0f },
                { "bp_low_cut", 120.0f },
                { "bp_high_cut", 18000.0f },
                { "bp_low_res", 0.0f },
                { "bp_high_res", 0.0f },
                { "bp_br_mix", 0.2f },
                { "dist_bypass", 0.0f },
                { "dist_mode", 0.0f }, // Classic
                { "dist_mech", 0.0f },
                { "dist_tube", 0.35f },
                { "dist_bit", 0.0f },
                { "dist_xcita", 0.45f },
                { "mm_bypass", 1.0f },
                { "flg_bypass", 0.0f },
                { "flg_rate", 0.2f },
                { "flg_amount", 0.2f },
                { "meq_bypass", 0.0f },
                { "meq_pre", 0.0f },
                { "meq_tune", 110.0f },
                { "meq_amount", 0.3f },
                { "meq_phat", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.35f },
                { "comp_phat", 0.0f },
                { "master_clip", 1.0f },
                { "master_mono_bass", 1.0f },
                { "master_mix", 0.85f },
                { "master_vol", 0.0f }
            }
        },
        // 13. Raw Screech Comb Screamer
        {
            "Raw Screech Comb Screamer",
            {
                { "bp_bypass", 0.0f },
                { "bp_low_cut", 300.0f },
                { "bp_high_cut", 8500.0f },
                { "bp_br_mix", 0.0f },
                { "dist_bypass", 0.0f },
                { "dist_mode", 2.0f }, // Wavefold
                { "dist_mech", 0.85f },
                { "dist_tube", 0.5f },
                { "mm_bypass", 0.0f },
                { "mm_type", 8.0f }, // Comb +
                { "mm_cutoff", 880.0f },
                { "mm_res", 0.85f },
                { "mm_env_depth", 0.65f },
                { "flg_bypass", 1.0f },
                { "meq_bypass", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.6f },
                { "comp_phat", 1.0f },
                { "master_clip", 1.0f },
                { "master_mono_bass", 1.0f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 14. Reverse Bass Pump
        {
            "Reverse Bass Pump",
            {
                { "bp_bypass", 0.0f },
                { "bp_low_cut", 35.0f },
                { "bp_high_cut", 5000.0f },
                { "bp_br_mix", 0.4f },
                { "dist_bypass", 0.0f },
                { "dist_mode", 1.0f }, // Hard Clip
                { "dist_tube", 0.6f },
                { "dist_mech", 0.4f },
                { "mm_bypass", 0.0f },
                { "mm_type", 0.0f }, // LP 12dB
                { "mm_cutoff", 1600.0f },
                { "mm_res", 0.5f },
                { "flg_bypass", 1.0f },
                { "meq_bypass", 0.0f },
                { "meq_pre", 1.0f },
                { "meq_tune", 65.0f },
                { "meq_amount", 0.7f },
                { "meq_phat", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.65f },
                { "comp_phat", 1.0f },
                { "lfo1_sync", 1.0f },
                { "lfo1_sync_div", 4.0f }, // 1/4 note
                { "lfo1_wave", 3.0f }, // Saw Up
                { "lfo1_target", 21.0f }, // Master Vol
                { "lfo1_depth", 0.7f },
                { "master_clip", 1.0f },
                { "master_mono_bass", 1.0f },
                { "master_mix", 1.0f },
                { "master_vol", 0.0f }
            }
        },
        // 15. Big Room Festival Kick
        {
            "Big Room Festival Kick",
            {
                { "bp_bypass", 0.0f },
                { "bp_low_cut", 50.0f },
                { "bp_high_cut", 12000.0f },
                { "bp_br_mix", 0.5f },
                { "meq_bypass", 0.0f },
                { "meq_pre", 0.0f },
                { "meq_tune", 80.0f },
                { "meq_amount", 0.75f },
                { "meq_phat", 1.0f },
                { "dist_bypass", 0.0f },
                { "dist_mode", 1.0f }, // Hard Clip
                { "dist_tube", 0.65f },
                { "dist_mech", 0.5f },
                { "dist_xcita", 0.35f },
                { "mm_bypass", 1.0f },
                { "flg_bypass", 1.0f },
                { "comp_bypass", 0.0f },
                { "comp_amount", 0.85f },
                { "comp_release", 70.0f },
                { "comp_phat", 1.0f },
                { "master_clip", 1.0f },
                { "master_mono_bass", 1.0f },
                { "master_mix", 0.85f },
                { "master_vol", 0.0f }
            }
        }
    };
    return presets;
}

} // namespace wvzz
