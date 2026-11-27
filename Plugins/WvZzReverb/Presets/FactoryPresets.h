#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <utility>

namespace wvzz::reverbfx {

struct Preset
{
    const char* name;
    // Parameter id -> value in real units. Parameters not listed are set to their default
    // when the preset loads, so a preset never depends on the one loaded before it.
    // rev_mode: 0 Plate, 1 Hall, 2 Shimmer.
    // rev_interval: 0 = +1 octave, 1 = +fifth, 2 = +octave+fifth, 3 = +2 octaves, 4 = -1 octave.
    std::vector<std::pair<const char*, float>> values;
};

inline const std::vector<Preset>& getFactoryPresets()
{
    static const std::vector<Preset> presets = {
        {
            "Bright Vocal Plate",
            {{
                { "rev_mode", 0.0f }, { "rev_predelay", 35.0f }, { "rev_size", 0.45f }, { "rev_decay", 1.8f },
                { "rev_lodamp", 180.0f }, { "rev_hidamp", 11000.0f }, { "rev_diffusion", 0.85f }, { "rev_mod", 0.2f },
                { "rev_mix", 0.25f }, { "er_level", 0.3f }, { "er_size", 0.3f }
            }}
        },
        {
            "Tight Drum Plate",
            {{
                { "rev_mode", 0.0f }, { "rev_predelay", 8.0f }, { "rev_size", 0.25f }, { "rev_decay", 0.8f },
                { "rev_lodamp", 250.0f }, { "rev_hidamp", 8000.0f }, { "rev_diffusion", 0.9f }, { "rev_mod", 0.1f },
                { "rev_mix", 0.2f }, { "er_level", 0.45f }, { "er_size", 0.2f }
            }}
        },
        {
            "Dark Studio Plate",
            {{
                { "rev_mode", 0.0f }, { "rev_predelay", 20.0f }, { "rev_size", 0.6f }, { "rev_decay", 2.4f },
                { "rev_lodamp", 120.0f }, { "rev_hidamp", 3500.0f }, { "rev_diffusion", 0.8f }, { "rev_mod", 0.3f },
                { "rev_mix", 0.3f }, { "er_level", 0.25f }
            }}
        },
        {
            "Small Room",
            {{
                { "rev_mode", 1.0f }, { "rev_predelay", 5.0f }, { "rev_size", 0.1f }, { "rev_decay", 0.6f },
                { "rev_lodamp", 150.0f }, { "rev_hidamp", 7000.0f }, { "rev_diffusion", 0.6f }, { "rev_mod", 0.1f },
                { "rev_mix", 0.22f }, { "er_level", 0.7f }, { "er_size", 0.25f }, { "er_diffusion", 0.4f }
            }}
        },
        {
            "Concert Hall",
            {{
                { "rev_mode", 1.0f }, { "rev_predelay", 30.0f }, { "rev_size", 0.7f }, { "rev_decay", 3.2f },
                { "rev_lodamp", 90.0f }, { "rev_hidamp", 6000.0f }, { "rev_diffusion", 0.7f }, { "rev_mod", 0.25f },
                { "rev_mix", 0.3f }, { "er_level", 0.5f }, { "er_size", 0.7f }
            }}
        },
        {
            "Cathedral",
            {{
                { "rev_mode", 1.0f }, { "rev_predelay", 60.0f }, { "rev_size", 1.0f }, { "rev_decay", 8.0f },
                { "rev_lodamp", 60.0f }, { "rev_hidamp", 4500.0f }, { "rev_diffusion", 0.75f }, { "rev_mod", 0.35f },
                { "rev_mix", 0.35f }, { "er_level", 0.4f }, { "er_size", 1.0f }
            }}
        },
        {
            "Infinite Pad Wash",
            {{
                { "rev_mode", 1.0f }, { "rev_predelay", 0.0f }, { "rev_size", 0.9f }, { "rev_decay", 18.0f },
                { "rev_lodamp", 200.0f }, { "rev_hidamp", 5000.0f }, { "rev_diffusion", 1.0f }, { "rev_mod", 0.6f },
                { "rev_mix", 0.5f }, { "er_level", 0.1f }
            }}
        },
        {
            "Octave Shimmer",
            {{
                { "rev_mode", 2.0f }, { "rev_predelay", 25.0f }, { "rev_size", 0.75f }, { "rev_decay", 5.0f },
                { "rev_lodamp", 200.0f }, { "rev_hidamp", 9000.0f }, { "rev_diffusion", 0.8f }, { "rev_mod", 0.4f },
                { "rev_shimmer", 0.6f }, { "rev_interval", 0.0f }, { "rev_mix", 0.4f }, { "er_level", 0.2f }
            }}
        },
        {
            "Fifth Shimmer Choir",
            {{
                { "rev_mode", 2.0f }, { "rev_predelay", 40.0f }, { "rev_size", 0.85f }, { "rev_decay", 7.0f },
                { "rev_lodamp", 250.0f }, { "rev_hidamp", 7000.0f }, { "rev_diffusion", 0.9f }, { "rev_mod", 0.5f },
                { "rev_shimmer", 0.75f }, { "rev_interval", 2.0f }, { "rev_mix", 0.45f }, { "er_level", 0.15f }
            }}
        },
        {
            "Sub Octave Cavern",
            {{
                { "rev_mode", 2.0f }, { "rev_predelay", 15.0f }, { "rev_size", 1.0f }, { "rev_decay", 6.0f },
                { "rev_lodamp", 40.0f }, { "rev_hidamp", 3000.0f }, { "rev_diffusion", 0.7f }, { "rev_mod", 0.3f },
                { "rev_shimmer", 0.5f }, { "rev_interval", 4.0f }, { "rev_width", 0.8f }, { "rev_mix", 0.4f }, { "er_level", 0.3f }
            }}
        }
    };
    return presets;
}

} // namespace wvzz::reverbfx
