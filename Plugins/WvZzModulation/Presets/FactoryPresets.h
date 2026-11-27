#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <utility>

namespace wvzz::modulationfx {

struct Preset
{
    const char* name;
    // Parameter id -> value in real units. Parameters not listed are set to their default
    // when the preset loads, so a preset never depends on the one loaded before it.
    std::vector<std::pair<const char*, float>> values;
};

inline const std::vector<Preset>& getFactoryPresets()
{
    static const std::vector<Preset> presets = {
        {
            "Lush Stereo Chorus",
            {{
                { "chor_bypass", 0.0f }, { "chor_rate", 0.6f }, { "chor_depth", 0.45f }, { "chor_delay", 14.0f },
                { "chor_voices", 2.0f }, { "chor_spread", 0.8f }, { "chor_feedback", 0.0f }, { "chor_mix", 0.5f }
            }}
        },
        {
            "Dimension Wide",
            {{
                { "chor_bypass", 0.0f }, { "chor_rate", 0.25f }, { "chor_depth", 0.22f }, { "chor_delay", 8.0f },
                { "chor_voices", 3.0f }, { "chor_spread", 1.0f }, { "chor_feedback", 0.0f }, { "chor_mix", 0.35f }
            }}
        },
        {
            "Vintage 4-Stage Phaser",
            {{
                { "phs_bypass", 0.0f }, { "phs_rate", 0.4f }, { "phs_depth", 0.65f }, { "phs_center", 700.0f },
                { "phs_stages", 1.0f }, { "phs_feedback", 0.45f }, { "phs_stereo", 0.25f }, { "phs_mix", 0.5f }
            }}
        },
        {
            "Deep 12-Stage Sweep",
            {{
                { "phs_bypass", 0.0f }, { "phs_rate", 0.08f }, { "phs_depth", 0.9f }, { "phs_center", 1100.0f },
                { "phs_stages", 4.0f }, { "phs_feedback", 0.7f }, { "phs_stereo", 0.5f }, { "phs_mix", 0.5f }
            }}
        },
        {
            "Jet Flanger",
            {{
                { "flg_bypass", 0.0f }, { "flg_rate", 0.15f }, { "flg_amount", 0.5f }, { "flg_feedback", 0.85f },
                { "flg_depth", 2.2f }, { "flg_center", 2.6f }, { "flg_tzero", 0.0f }
            }}
        },
        {
            "Through-Zero Tape Flange",
            {{
                { "flg_bypass", 0.0f }, { "flg_rate", 0.1f }, { "flg_amount", 0.5f }, { "flg_feedback", -0.3f },
                { "flg_depth", 3.0f }, { "flg_center", 3.0f }, { "flg_tzero", 1.0f }
            }}
        },
        {
            "Harmonic Tremolo Shimmer",
            {{
                { "trem_bypass", 0.0f }, { "trem_rate", 5.5f }, { "trem_depth", 0.7f }, { "trem_shape", 0.0f },
                { "trem_mode", 1.0f }, { "trem_stereo", 0.0f }, { "trem_xover", 700.0f }
            }}
        },
        {
            "Auto-Pan Pulse",
            {{
                { "trem_bypass", 0.0f }, { "trem_rate", 2.0f }, { "trem_depth", 0.85f }, { "trem_shape", 1.0f },
                { "trem_mode", 0.0f }, { "trem_stereo", 1.0f }
            }}
        },
        {
            "Ensemble Strings",
            {{
                { "chor_bypass", 0.0f }, { "chor_rate", 0.9f }, { "chor_depth", 0.35f }, { "chor_delay", 18.0f },
                { "chor_voices", 3.0f }, { "chor_spread", 0.9f }, { "chor_feedback", 0.15f }, { "chor_mix", 0.55f },
                { "phs_bypass", 0.0f }, { "phs_rate", 0.12f }, { "phs_depth", 0.4f }, { "phs_center", 1500.0f },
                { "phs_stages", 0.0f }, { "phs_feedback", 0.2f }, { "phs_stereo", 0.5f }, { "phs_mix", 0.3f }
            }}
        },
        {
            "Psychedelic Swirl",
            {{
                { "chor_bypass", 0.0f }, { "chor_rate", 0.35f }, { "chor_depth", 0.5f }, { "chor_delay", 10.0f },
                { "chor_voices", 1.0f }, { "chor_spread", 0.7f }, { "chor_feedback", 0.2f }, { "chor_mix", 0.4f },
                { "phs_bypass", 0.0f }, { "phs_rate", 0.2f }, { "phs_depth", 0.8f }, { "phs_center", 900.0f },
                { "phs_stages", 2.0f }, { "phs_feedback", 0.6f }, { "phs_stereo", 1.0f }, { "phs_mix", 0.5f },
                { "flg_bypass", 0.0f }, { "flg_rate", 0.07f }, { "flg_amount", 0.35f }, { "flg_feedback", 0.6f },
                { "flg_depth", 1.8f }, { "flg_center", 3.5f }, { "flg_tzero", 0.0f },
                { "trem_bypass", 0.0f }, { "trem_rate", 3.0f }, { "trem_depth", 0.35f }, { "trem_shape", 0.0f },
                { "trem_mode", 1.0f }, { "trem_stereo", 0.5f }, { "trem_xover", 900.0f }
            }}
        }
    };
    return presets;
}

} // namespace wvzz::modulationfx
