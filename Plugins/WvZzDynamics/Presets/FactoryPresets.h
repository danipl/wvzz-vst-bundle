#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <utility>

namespace wvzz::dynamics {

struct Preset
{
    const char* name;
    // Parameter values key-value pairs
    std::vector<std::pair<const char*, float>> values;
};

inline const std::vector<Preset>& getFactoryPresets()
{
    static const std::vector<Preset> presets = {
        {
            "Euphoric Lead Wall",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 110.0f }, { "det_sub_tight", 0.3f },
                { "trans_attack", 0.4f }, { "trans_sustain", 0.2f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.55f }, { "ott_up_drive", 0.70f }, { "ott_down_drive", 0.50f }, { "ott_sheen", 6.0f },
                { "comp_thresh", -18.0f }, { "comp_ratio", 4.0f }, { "comp_mode", 1.0f }, { "comp_mix", 0.8f },
                { "pump_amount", 0.5f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 4.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 1.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Tok & Tail Kick Glue",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 85.0f }, { "det_sub_tight", 0.6f },
                { "trans_attack", 0.75f }, { "trans_sustain", -0.2f }, { "trans_speed", 1.0f },
                { "ott_depth", 0.30f }, { "ott_up_drive", 0.40f }, { "ott_down_drive", 0.70f }, { "ott_sheen", 0.0f },
                { "comp_thresh", -14.0f }, { "comp_ratio", 6.0f }, { "comp_mode", 0.0f }, { "comp_phat", 1.0f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 6.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 0.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Reverse Bass Pumper",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 130.0f }, { "det_sub_tight", 0.2f },
                { "trans_attack", 0.1f }, { "trans_sustain", 0.5f }, { "trans_speed", 1.0f },
                { "ott_depth", 0.45f }, { "ott_up_drive", 0.60f }, { "ott_down_drive", 0.40f }, { "ott_sheen", 2.0f },
                { "comp_thresh", -16.0f }, { "comp_ratio", 4.0f }, { "comp_mode", 1.0f }, { "comp_mix", 0.9f },
                { "pump_amount", 0.95f }, { "pump_sync_div", 2.0f }, { "pump_shape", 2.0f }, { "pump_curve", 0.65f },
                { "clip_drive", 3.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 2.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Screech OTT Maximizer",
            {{
                { "det_sub_lock", 0.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.0f },
                { "trans_attack", 0.6f }, { "trans_sustain", 0.4f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.85f }, { "ott_up_drive", 0.90f }, { "ott_down_drive", 0.80f }, { "ott_sheen", 8.0f },
                { "comp_thresh", -24.0f }, { "comp_ratio", 8.0f }, { "comp_mode", 2.0f }, { "comp_phat", 1.0f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 8.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 2.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 0.0f }
            }}
        },
        {
            "Rawstyle Kick Clipper",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 95.0f }, { "det_sub_tight", 0.5f },
                { "trans_attack", 0.9f }, { "trans_sustain", -0.3f }, { "trans_speed", 1.0f },
                { "ott_depth", 0.40f }, { "ott_up_drive", 0.50f }, { "ott_down_drive", 0.80f }, { "ott_sheen", 0.0f },
                { "comp_thresh", -12.0f }, { "comp_ratio", 10.0f }, { "comp_mode", 2.0f }, { "comp_phat", 1.0f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 10.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 2.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Mainstage Drop Bus",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 100.0f }, { "det_sub_tight", 0.2f },
                { "trans_attack", 0.3f }, { "trans_sustain", 0.1f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.45f }, { "ott_up_drive", 0.50f }, { "ott_down_drive", 0.60f }, { "ott_sheen", 4.0f },
                { "comp_thresh", -16.0f }, { "comp_ratio", 3.5f }, { "comp_mode", 1.0f }, { "comp_mix", 0.75f },
                { "pump_amount", 0.4f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.45f },
                { "clip_drive", 3.5f }, { "clip_ceiling", -0.1f }, { "clip_mode", 0.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Big Room Kick Snap",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 80.0f }, { "det_sub_tight", 0.4f },
                { "trans_attack", 0.7f }, { "trans_sustain", -0.1f }, { "trans_speed", 1.0f },
                { "ott_depth", 0.25f }, { "ott_up_drive", 0.30f }, { "ott_down_drive", 0.60f }, { "ott_sheen", 0.0f },
                { "comp_thresh", -12.0f }, { "comp_ratio", 5.0f }, { "comp_mode", 1.0f }, { "comp_mix", 1.0f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 4.5f }, { "clip_ceiling", -0.1f }, { "clip_mode", 0.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Festival Sub Lock",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.75f },
                { "trans_attack", 0.0f }, { "trans_sustain", -0.4f }, { "trans_speed", 1.0f },
                { "ott_depth", 0.20f }, { "ott_up_drive", 0.20f }, { "ott_down_drive", 0.50f }, { "ott_sheen", 0.0f },
                { "comp_thresh", -15.0f }, { "comp_ratio", 4.0f }, { "comp_mode", 0.0f }, { "comp_mix", 1.0f },
                { "pump_amount", 0.6f }, { "pump_sync_div", 2.0f }, { "pump_shape", 1.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 2.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 1.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Pluck Transient Pop",
            {{
                { "det_sub_lock", 0.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.0f },
                { "trans_attack", 0.85f }, { "trans_sustain", -0.5f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.40f }, { "ott_up_drive", 0.60f }, { "ott_down_drive", 0.40f }, { "ott_sheen", 5.0f },
                { "comp_thresh", -18.0f }, { "comp_ratio", 4.0f }, { "comp_mode", 2.0f }, { "comp_mix", 0.85f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 1.5f }, { "clip_ceiling", -0.1f }, { "clip_mode", 1.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 0.0f }
            }}
        },
        {
            "SuperSaw Hyper-OTT",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 120.0f }, { "det_sub_tight", 0.1f },
                { "trans_attack", 0.2f }, { "trans_sustain", 0.3f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.70f }, { "ott_up_drive", 0.80f }, { "ott_down_drive", 0.60f }, { "ott_sheen", 7.0f },
                { "comp_thresh", -20.0f }, { "comp_ratio", 4.0f }, { "comp_mode", 1.0f }, { "comp_mix", 0.70f },
                { "pump_amount", 0.7f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.45f },
                { "clip_drive", 3.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 1.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Clap & Snare Smash",
            {{
                { "det_sub_lock", 0.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.0f },
                { "trans_attack", 0.65f }, { "trans_sustain", 0.4f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.50f }, { "ott_up_drive", 0.70f }, { "ott_down_drive", 0.60f }, { "ott_sheen", 4.0f },
                { "comp_thresh", -16.0f }, { "comp_ratio", 6.0f }, { "comp_mode", 2.0f }, { "comp_mix", 1.0f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 5.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 0.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 0.0f }
            }}
        },
        {
            "Drum Bus VCA Glue",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 80.0f }, { "det_sub_tight", 0.2f },
                { "trans_attack", 0.25f }, { "trans_sustain", -0.1f }, { "trans_speed", 1.0f },
                { "ott_depth", 0.20f }, { "ott_up_drive", 0.25f }, { "ott_down_drive", 0.40f }, { "ott_sheen", 1.5f },
                { "comp_thresh", -14.0f }, { "comp_ratio", 3.0f }, { "comp_mode", 1.0f }, { "comp_mix", 0.85f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 2.5f }, { "clip_ceiling", -0.1f }, { "clip_mode", 0.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Hi-Hat Air & Sparkle",
            {{
                { "det_sub_lock", 0.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.0f },
                { "trans_attack", 0.4f }, { "trans_sustain", -0.4f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.45f }, { "ott_up_drive", 0.50f }, { "ott_down_drive", 0.40f }, { "ott_sheen", 9.0f },
                { "comp_thresh", -20.0f }, { "comp_ratio", 4.0f }, { "comp_mode", 0.0f }, { "comp_mix", 0.9f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 1.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 1.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 0.0f }
            }}
        },
        {
            "Tight Gate Percussion",
            {{
                { "det_sub_lock", 0.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.0f },
                { "trans_attack", 0.7f }, { "trans_sustain", -0.8f }, { "trans_speed", 0.0f }, { "trans_gate", -36.0f },
                { "ott_depth", 0.20f }, { "ott_up_drive", 0.20f }, { "ott_down_drive", 0.50f }, { "ott_sheen", 0.0f },
                { "comp_thresh", -15.0f }, { "comp_ratio", 4.0f }, { "comp_mode", 2.0f }, { "comp_mix", 1.0f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 3.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 0.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 0.0f }
            }}
        },
        {
            "Euphoric Vocal Up-Lift",
            {{
                { "det_sub_lock", 0.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.0f },
                { "trans_attack", 0.1f }, { "trans_sustain", 0.35f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.60f }, { "ott_up_drive", 0.75f }, { "ott_down_drive", 0.50f }, { "ott_sheen", 5.0f },
                { "comp_thresh", -22.0f }, { "comp_ratio", 3.0f }, { "comp_mode", 0.0f }, { "comp_mix", 0.85f },
                { "pump_amount", 0.3f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 2.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 1.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 0.0f }
            }}
        },
        {
            "Piano & Pluck Leveler",
            {{
                { "det_sub_lock", 0.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.0f },
                { "trans_attack", 0.4f }, { "trans_sustain", 0.2f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.35f }, { "ott_up_drive", 0.50f }, { "ott_down_drive", 0.40f }, { "ott_sheen", 3.0f },
                { "comp_thresh", -18.0f }, { "comp_ratio", 2.5f }, { "comp_mode", 0.0f }, { "comp_mix", 0.9f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 1.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 1.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 0.0f }
            }}
        },
        {
            "Sidechain Pad Swell",
            {{
                { "det_sub_lock", 0.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.0f },
                { "trans_attack", -0.3f }, { "trans_sustain", 0.4f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.50f }, { "ott_up_drive", 0.60f }, { "ott_down_drive", 0.30f }, { "ott_sheen", 4.0f },
                { "comp_thresh", -20.0f }, { "comp_ratio", 3.0f }, { "comp_mode", 0.0f }, { "comp_mix", 1.0f },
                { "pump_amount", 0.85f }, { "pump_sync_div", 2.0f }, { "pump_shape", 3.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 2.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 1.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 0.0f }
            }}
        },
        {
            "Club Loudness Maximizer",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.2f },
                { "trans_attack", 0.2f }, { "trans_sustain", 0.0f }, { "trans_speed", 1.0f },
                { "ott_depth", 0.30f }, { "ott_up_drive", 0.40f }, { "ott_down_drive", 0.60f }, { "ott_sheen", 2.0f },
                { "comp_thresh", -12.0f }, { "comp_ratio", 4.0f }, { "comp_mode", 1.0f }, { "comp_mix", 0.8f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 6.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 0.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Parallel Bus Slam",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 90.0f }, { "det_sub_tight", 0.3f },
                { "trans_attack", 0.5f }, { "trans_sustain", 0.3f }, { "trans_speed", 1.0f },
                { "ott_depth", 0.60f }, { "ott_up_drive", 0.70f }, { "ott_down_drive", 0.80f }, { "ott_sheen", 3.0f },
                { "comp_thresh", -24.0f }, { "comp_ratio", 8.0f }, { "comp_mode", 2.0f }, { "comp_mix", 0.50f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 7.0f }, { "clip_ceiling", -0.1f }, { "clip_mode", 0.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        },
        {
            "Clean Mastering Glue",
            {{
                { "det_sub_lock", 1.0f }, { "det_sub_xover", 80.0f }, { "det_sub_tight", 0.1f },
                { "trans_attack", 0.1f }, { "trans_sustain", 0.0f }, { "trans_speed", 0.0f },
                { "ott_depth", 0.15f }, { "ott_up_drive", 0.20f }, { "ott_down_drive", 0.30f }, { "ott_sheen", 1.0f },
                { "comp_thresh", -10.0f }, { "comp_ratio", 2.0f }, { "comp_mode", 1.0f }, { "comp_mix", 1.0f },
                { "pump_amount", 0.0f }, { "pump_sync_div", 2.0f }, { "pump_shape", 0.0f }, { "pump_curve", 0.5f },
                { "clip_drive", 1.5f }, { "clip_ceiling", -0.1f }, { "clip_mode", 1.0f },
                { "master_mix", 1.0f }, { "master_vol", 0.0f }, { "master_mono_bass", 1.0f }
            }}
        }
    };
    return presets;
}

} // namespace wvzz::dynamics
