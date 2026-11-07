#pragma once

#include "IHelpSystem.h"
#include <unordered_map>

namespace camelphat::ui {

/**
 * @brief Repository of plain-language contextual explanations for all modules and controls.
 * Implements IHelpContentProvider interface.
 */
class HelpContentRepository : public IHelpContentProvider
{
public:
    HelpContentRepository()
    {
        initTopics();
    }

    ~HelpContentRepository() override = default;

    [[nodiscard]] HelpTopic getTopic(const juce::String& topicId) const override
    {
        auto it = topics.find(topicId);
        if (it != topics.end())
            return it->second;

        return {
            "Module Information",
            "Contextual guidance for this component is currently active.",
            "Signal Processing Path",
            { "Use the rotary knobs and toggles to adjust parameters." }
        };
    }

private:
    void initTopics()
    {
        topics["bp_filter"] = {
            "BP Filter (Bandpass + Band-Reject Split)",
            "Splits the incoming audio into a passband (sent to Distortion and MM Filter) and a rejected band (summed back in post-MM Filter via BR Mix) using dual 12 dB/oct ZDF SVFs.",
            "Stage 1: Band Isolation & Rejection",
            {
                "Low Cut: High-pass cutoff frequency (20 Hz - 20 kHz).",
                "High Cut: Low-pass cutoff frequency (20 Hz - 20 kHz).",
                "Link: Locks log-frequency delta between Low and High Cut.",
                "Low/High Res: Independent resonance at the cutoff points (Q: 0.707 to 12.0).",
                "BR Mix: Blends clean rejected frequencies back post-MM Filter.",
                "Bypass: Completely bypasses the filter and split."
            }
        };

        topics["distortion"] = {
            "Distortion (4 Parallel Circuits + 4x Oversampling)",
            "Four characterful saturation circuits with 4x polyphase IIR oversampling and 10 Hz DC blocking.",
            "Stage 2: Parallel Waveshaping",
            {
                "Mech: Aggressive mechanical grit with 1.2 kHz pre-emphasis and asymmetric sine wavefolding.",
                "Tube: Smooth analog-style even/odd harmonic saturation (exp drive 1 to 15).",
                "Bitcrusher: Quantization (16 to 4 bit) and sample-and-hold (1x to 32x) at base rate.",
                "Xcita: High-frequency harmonic exciter with full-wave rectification above 2.8 kHz."
            }
        };

        topics["mm_filter"] = {
            "MM Filter (Multi-Mode Filter + Envelope Follower)",
            "12 distinct filter topologies with envelope follower modulation tapped from the pre-BP input.",
            "Stage 3: Multi-Mode Tone Shaping",
            {
                "Filter Type: Selects between 12 modes (LP/BP/HP 12/24dB, Notch, Peak, Comb +/-, RingMod, Vowel).",
                "Cutoff & Res: Primary frequency and feedback controls up to self-oscillation.",
                "Mix: Wet/dry blend of the MM Filter output.",
                "Env Attack/Release: Ballistics for envelope follower tracking.",
                "Env Mod: Bipolar modulation depth on cutoff (+/- 6 octaves)."
            }
        };

        topics["flanger"] = {
            "Flanger (Stereo Modulated)",
            "Stereo modulated delay line with 3rd-order Lagrange interpolation, 90-degree quadrature LFO, and 0.55 fixed feedback.",
            "Stage 4: Stereo Phasing & Comb Filtering",
            {
                "Rate: LFO sweep speed (0.01 to 20.0 Hz, default 0.4 Hz).",
                "Amount: Wet/dry mix of the flanged signal.",
                "Bypass: Completely bypasses the flanger delay line."
            }
        };

        topics["magic_eq"] = {
            "Magic EQ (Resonant Low-End Enhancer)",
            "Pultec-style resonant bass enhancer with complimentary anti-mud dip at 2 * tune.",
            "Stage 5: Low-End Enhancement",
            {
                "Amount: Intensity of low-end resonant boost (0 to +12 dB).",
                "Tune: Center frequency of the sub/bass enhancement (60 to 120 Hz).",
                "Phat Mode (P): Places tanh clipper inside resonant state and adds +3 dB air shelf at 8 kHz."
            }
        };

        topics["compressor"] = {
            "Compressor (Optical Dynamics & Glue)",
            "Studio-grade optical feed-forward compressor with dual-stage release ballistics, dynamic auto-makeup gain, and Anti-Derivative Anti-Aliasing (ADAA) saturation.",
            "Stage 6: Dynamics & Parallel Compression",
            {
                "Thresh: Sets the compression threshold (-48 to 0 dB).",
                "Ratio: Sets compression slope (1.5:1 gentle leveling to 20:1 brickwall limiting).",
                "Attack: Sets onset reaction time (0.1 ms fast clamping to 50 ms transient punch).",
                "Release: Dual-stage optical decay timing with fast initial recovery (~50 ms) and program-dependent tail (10 to 1500 ms).",
                "Mix: Dry/wet parallel compression blend (0% dry to 100% wet).",
                "Phat Mode (P): Engages -0.1 dBFS ceiling soft-clipping with zero-latency 1st-order ADAA anti-aliasing.",
                "GR Meter: Wide horizontal gain-reduction display with analog ballistics, calibrated dB ticks, and floating peak-hold."
            }
        };

        topics["master"] = {
            "Master Section & Utility",
            "Latency-compensated dry/wet parallel blend, output volume trim, Dice randomizer, and peak/RMS VU metering.",
            "Stage 7: Output Level & Balance",
            {
                "Input Gain: Input trim (-24 to +24 dB).",
                "Master Mix: Global dry/wet parallel blend with oversampling latency compensation.",
                "Master Vol: Output volume trim (-36 to +12 dB).",
                "Dice: Smart musical randomizer avoiding runaway resonances."
            }
        };

        topics["xy_pad"] = {
            "X/Y Controller Pad",
            "2D macro surface applying additive bipolar or unipolar offsets to any parameters from the 21-item Modulatable Targets Enum.",
            "Macro Gesture Surface",
            {
                "X & Y Axes: Horizontal and vertical macro offsets (0.0 to 1.0).",
                "Target Selectors: Assign any target from the modulation routing matrix.",
                "Depth Knobs: Configures bipolar modulation depth (-1.0 to +1.0)."
            }
        };

        topics["lfo"] = {
            "LFO 1 & LFO 2 Modulation Panels",
            "Two independent modulation oscillators computed at control rate with 7 waveforms and Free / Tempo Sync modes.",
            "Modulation Oscillators",
            {
                "Waveforms: Sine, Triangle, Saw Down, Saw Up, Square, Sample & Hold, Smooth Random.",
                "Tempo Sync: Free (0.02 to 30 Hz) or Host Sync (4/1 to 1/32, Dotted & Triplet).",
                "Target & Depth: Modulates any parameter in the 21-target matrix (including LFO 2 Rate from LFO 1)."
            }
        };
    }

    std::unordered_map<juce::String, HelpTopic> topics;
};

} // namespace camelphat::ui
