#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <utility>

namespace wvzz::delayfx {

struct Preset
{
    const char* name;
    // Parameter id -> value in real units. Parameters not listed are set to their default
    // when the preset loads, so a preset never depends on the one loaded before it.
    // dly_mode: 0 Clean, 1 Tape, 2 BBD, 3 Granular. dly_routing: 0 Stereo, 1 Ping-Pong.
    // dly_sync_div: 2 = 1/2, 4 = 1/4D, 5 = 1/4, 7 = 1/8D, 8 = 1/8, 11 = 1/16.
    std::vector<std::pair<const char*, float>> values;
};

inline const std::vector<Preset>& getFactoryPresets()
{
    static const std::vector<Preset> presets = {
        {
            "Clean Quarter Note",
            {{
                { "dly_mode", 0.0f }, { "dly_sync", 1.0f }, { "dly_sync_div", 5.0f }, { "dly_routing", 0.0f },
                { "dly_feedback", 0.35f }, { "dly_hp", 120.0f }, { "dly_lp", 9000.0f }, { "dly_mix", 0.3f }
            }}
        },
        {
            "Dotted Eighth Ping-Pong",
            {{
                { "dly_mode", 0.0f }, { "dly_sync", 1.0f }, { "dly_sync_div", 7.0f }, { "dly_routing", 1.0f },
                { "dly_feedback", 0.5f }, { "dly_hp", 200.0f }, { "dly_lp", 7000.0f }, { "dly_duck", 0.4f }, { "dly_mix", 0.32f }
            }}
        },
        {
            "Slapback Tape",
            {{
                { "dly_mode", 1.0f }, { "dly_time", 115.0f }, { "dly_feedback", 0.12f }, { "dly_wow", 0.25f },
                { "dly_drive", 0.45f }, { "dly_hp", 90.0f }, { "dly_lp", 6500.0f }, { "dly_mix", 0.3f }
            }}
        },
        {
            "Worn Tape Echo",
            {{
                { "dly_mode", 1.0f }, { "dly_time", 380.0f }, { "dly_feedback", 0.62f }, { "dly_wow", 0.7f },
                { "dly_drive", 0.6f }, { "dly_hp", 150.0f }, { "dly_lp", 4200.0f }, { "dly_offset", 12.0f }, { "dly_mix", 0.35f }
            }}
        },
        {
            "Dark BBD Chorus Echo",
            {{
                { "dly_mode", 2.0f }, { "dly_time", 290.0f }, { "dly_feedback", 0.55f }, { "dly_wow", 0.6f },
                { "dly_hp", 100.0f }, { "dly_lp", 5000.0f }, { "dly_offset", 8.0f }, { "dly_mix", 0.38f }
            }}
        },
        {
            "Dub Feedback Wash",
            {{
                { "dly_mode", 2.0f }, { "dly_sync", 1.0f }, { "dly_sync_div", 4.0f }, { "dly_routing", 1.0f },
                { "dly_feedback", 0.92f }, { "dly_hp", 300.0f }, { "dly_lp", 3200.0f }, { "dly_diffusion", 0.5f },
                { "dly_duck", 0.5f }, { "dly_mix", 0.4f }
            }}
        },
        {
            "Diffused Ambient Trail",
            {{
                { "dly_mode", 0.0f }, { "dly_time", 620.0f }, { "dly_feedback", 0.72f }, { "dly_diffusion", 1.0f },
                { "dly_hp", 180.0f }, { "dly_lp", 6000.0f }, { "dly_offset", 25.0f }, { "dly_duck", 0.3f }, { "dly_mix", 0.42f }
            }}
        },
        {
            "Ducked Vocal Throw",
            {{
                { "dly_mode", 0.0f }, { "dly_sync", 1.0f }, { "dly_sync_div", 2.0f }, { "dly_routing", 1.0f },
                { "dly_feedback", 0.45f }, { "dly_hp", 250.0f }, { "dly_lp", 8000.0f }, { "dly_duck", 0.9f }, { "dly_mix", 0.4f }
            }}
        },
        {
            "Granular Cloud",
            {{
                { "dly_mode", 3.0f }, { "dly_time", 450.0f }, { "dly_feedback", 0.6f }, { "dly_diffusion", 0.6f },
                { "dly_hp", 150.0f }, { "dly_lp", 9000.0f }, { "dly_mix", 0.45f },
                { "grain_size", 140.0f }, { "grain_density", 28.0f }, { "grain_pitch", 0.0f },
                { "grain_spray", 0.55f }, { "grain_reverse", 0.25f }, { "grain_spread", 0.9f }
            }}
        },
        {
            "Octave Shimmer Ladder",
            {{
                { "dly_mode", 3.0f }, { "dly_sync", 1.0f }, { "dly_sync_div", 8.0f }, { "dly_routing", 1.0f },
                { "dly_feedback", 0.55f }, { "dly_hp", 300.0f }, { "dly_lp", 10000.0f }, { "dly_duck", 0.3f }, { "dly_mix", 0.35f },
                { "grain_size", 90.0f }, { "grain_density", 30.0f }, { "grain_pitch", 12.0f },
                { "grain_spray", 0.05f }, { "grain_reverse", 0.0f }, { "grain_spread", 0.6f }
            }}
        }
    };
    return presets;
}

} // namespace wvzz::delayfx
