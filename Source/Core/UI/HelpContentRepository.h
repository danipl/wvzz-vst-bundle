#pragma once

#include "IHelpSystem.h"
#include <unordered_map>

namespace wvzz::ui {

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
                "Xcita: Dynamic high-frequency harmonic generation (3.5 kHz HPF + soft clipping).",
                "Bypass: Completely bypasses all saturation circuits."
            }
        };

        topics["mm_filter"] = {
            "MM Filter (12-Topology Multi-Mode Filter)",
            "Dual-SVF zero-delay feedback filter with 12 topologies (LP/BP/HP/Notch across 12, 18, 24 dB/oct), self-oscillating resonance, saturation inside feedback loop, and envelope follower.",
            "Stage 3: Spectral Sculpting",
            {
                "Type: Selects filter topology (LP12, LP18, LP24, BP12, BP24, HP12, HP24, Notch, etc.).",
                "Cutoff: Center/corner frequency (20 Hz - 20 kHz) with non-linear warping.",
                "Resonance: Resonance intensity (0.0 to 1.0) capable of clean self-oscillation.",
                "Mix: Dry/wet balance between pre-MM Filter audio and filtered signal.",
                "Env Follower: Pre-BP tapped sidechain envelope with Attack (0.1-200ms), Release (10-1000ms), and Depth (-1 to +1)."
            }
        };

        topics["flanger"] = {
            "Flanger (Stereo Quadrature Modulated Delay)",
            "A short delay that sweeps up and down. Mixed with the dry signal it makes a moving comb filter.",
            "Stereo Space & Movement",
            {
                "Rate: Sweep speed (0.01 to 20 Hz).",
                "Amount: Wet/dry mix (0.0 to 1.0).",
                "Feedback: Sends the delayed signal back in (-0.95 to 0.95). Negative values invert it. WvZz Modulation only.",
                "Depth: How far the delay sweeps around the center (0 to 5 ms). WvZz Modulation only.",
                "Center: The middle delay time (0.5 to 10 ms). WvZz Modulation only.",
                "Thru-Zero: Delays the dry signal by Center, so the sweep passes through zero delay. WvZz Modulation only.",
                "Bypass: Turns the flanger off."
            }
        };

        topics["chorus"] = {
            "Chorus (Multi-Voice Modulated Delay)",
            "Up to 4 delayed copies of the signal, each moving slightly in pitch and time. It makes one sound feel like several.",
            "Stage 1: Width & Thickness",
            {
                "Rate: Speed of the pitch movement (0.05 to 10 Hz).",
                "Depth: How far each voice moves (up to +/- 5 ms).",
                "Delay: Base delay of the voices (2 to 30 ms). Short is tight, long is wide.",
                "Spread: Left/right difference of the movement. 0 is mono.",
                "Feedback: Sends one voice back in for a resonant tone (0 to 0.7).",
                "Mix: Wet/dry balance.",
                "Voices: Number of delayed copies (1 to 4)."
            }
        };

        topics["phaser"] = {
            "Phaser (All-Pass Cascade)",
            "A chain of all-pass filters creates notches in the spectrum. An LFO moves the notches up and down.",
            "Stage 2: Sweeping Notches",
            {
                "Rate: Sweep speed (0.02 to 10 Hz).",
                "Depth: Sweep range around the center (up to +/- 2 octaves).",
                "Center: Middle frequency of the sweep (100 to 4000 Hz).",
                "Feedback: Makes the notches sharper (-0.95 to 0.95). Negative values change the tone.",
                "Stereo: LFO offset between left and right. 1.0 is 180 degrees.",
                "Mix: Wet/dry balance. 0.5 gives the deepest notches.",
                "Stages: Number of all-pass filters (2 to 12). More stages give more notches."
            }
        };

        topics["tremolo"] = {
            "Tremolo (Classic & Harmonic)",
            "Moves the volume up and down with an LFO. Harmonic mode moves lows and highs in opposite directions.",
            "Stage 4: Level Movement",
            {
                "Rate: Speed of the movement (0.1 to 20 Hz).",
                "Depth: How far the level drops (0 is off).",
                "Stereo: LFO offset between left and right. 1.0 turns the tremolo into an auto-pan.",
                "X-Over: Split frequency for Harmonic mode (200 to 2000 Hz).",
                "Shape: Sine, Triangle, or Square LFO.",
                "Mode: Classic changes the whole level. Harmonic alternates the low and high band."
            }
        };

        topics["lfo_phase_scope"] = {
            "LFO Phase Scope",
            "Shows where each effect LFO is in its cycle. The filled dot is the left channel, the ring is the right channel.",
            "Center Column: SCOPE tab",
            {
                "One lane per effect: Chorus, Phaser, Flanger, Tremolo.",
                "A dimmed lane means the effect is bypassed.",
                "The height of the curve follows the effect depth."
            }
        };

        topics["magic_eq"] = {
            "Magic EQ (Sub-Bass Harmonic Resonator)",
            "Dual-band equalizer with a sub-bass resonant peak and gentle high-frequency presence shelf.",
            "Stage 5: Low-End Weight & Sheen",
            {
                "Amount: Boost amount for sub-bass enhancement and presence.",
                "Tune: Pitch calibration for the sub-harmonic resonant peak.",
                "Phat: Engages warm second-order tube harmonic saturation on the low band."
            }
        };

        topics["compressor"] = {
            "Compressor (Optical Style Dynamics)",
            "Studio-grade optical feed-forward compressor with dual-stage release ballistics, dynamic auto-makeup gain, and Anti-Derivative Anti-Aliasing (ADAA) saturation.",
            "Stage 6: Dynamics & Parallel Compression",
            {
                "Thresh: Sets the compression threshold (-48 dB to 0 dB).",
                "Ratio: Sets compression slope (1.5:1 gentle leveling to 20:1 brickwall limiting).",
                "Attack: Sets onset reaction time (0.1 ms fast clamping to 50 ms transient punch).",
                "Release: Dual-stage optical decay timing with fast initial recovery (~50 ms) and program-dependent tail (10 ms to 1500 ms).",
                "Mix: Dry/wet parallel compression blend (0% dry to 100% wet).",
                "Phat: Engages -0.1 dBFS ceiling soft-clipping with zero-latency 1st-order ADAA anti-aliasing.",
                "GR Meter: Wide horizontal gain-reduction display with analog ballistics, calibrated dB ticks, and floating peak-hold."
            }
        };

        topics["master"] = {
            "Master Section (I/O, Recombination & Gain)",
            "Final output stage with master wet/dry balance, output level trim, and latency compensation.",
            "Stage 7: Output & Monitoring",
            {
                "Master Mix: Overall dry/wet balance of the entire plugin chain.",
                "Volume: Clean master output level trim (-24 dB to +12 dB).",
                "Master Bypass: Global clean bypass with click-free crossfade."
            }
        };

        topics["modulation"] = {
            "Modulation Section (Dual LFOs & Assignable X/Y Pad)",
            "Two tempo-syncable multi-waveform LFOs and an interactive X/Y Pad with crosshair control.",
            "Modulation Source",
            {
                "Waveform: Sine, Triangle, Saw Down, Saw Up, Square, Sample & Hold, Smooth Random.",
                "Sync / Free: Host tempo synchronization (4/1 to 1/32 including triplets and dotted).",
                "Target: Route to any of 21 destinations across the plugin.",
                "X/Y Pad: 2D macro control mapping X and Y axes independently to any destination."
            }
        };

        // --- Dynamics Modules Topics ---
        topics["dyn_detector"] = {
            "Sub & Detector (Linkwitz-Riley Sub Split & SC Filter)",
            "Isolates sub-bass below the crossover frequency into a locked channel and shapes the sidechain detector path with HPF and spectral tilt.",
            "Stage 1: Sub Lock & Detector Sculpting",
            {
                "Sub X-Over: 24 dB/oct crossover frequency (40 Hz - 300 Hz) to separate sub energy.",
                "Sub Lock: When active, routes sub-bass around transient & OTT stages, preventing mud.",
                "Sub Tight: Clamps sub sustain tails for punchy festival drops.",
                "SC HPF: High-pass filter for the compression detector (20 Hz - 500 Hz).",
                "SC Tilt: Spectral tilt filter (-6 dB to +6 dB/oct) weighting bass or high detector sensitivity.",
                "SC Listen: Auditions the filtered sidechain detector audio directly."
            }
        };

        topics["transient_shaper"] = {
            "Transient & Gate (Differential Snap & Noise Gate)",
            "Level-independent dual-envelope transient shaper with an integrated noise gate floor.",
            "Stage 2: Transient Snap & Gating",
            {
                "Attack: Boosts or cuts initial transient Tok (-12 dB to +12 dB).",
                "Sustain: Boosts or attenuates post-transient decay tails (-18 dB to +12 dB).",
                "Speed: Fast (leads and plucks) or Punch (longer kick envelope).",
                "Gate Floor: Silences noise and hiss between hits (-80 dB to 0 dB)."
            }
        };

        topics["multiband_ott"] = {
            "Hyper-OTT (3-Band Upward/Downward Compressor)",
            "Aggressive multiband dynamics processor lifting quiet detail and squashing peaks across Low, Mid, and High bands.",
            "Stage 3: Upward Density & Euphoric Sheen",
            {
                "Depth: Overall wet blend of the 3-band upward/downward engine (0% to 100%).",
                "Up Drive: Upward compression gain bringing quiet harmonic details forward.",
                "Down Drive: Downward compression clamping loud resonant peaks.",
                "Time: Ballistics scaling from 10% snap to 200% smooth leveling.",
                "High Sheen: High-frequency Euphoric air boost above 2.5 kHz (0 dB to +12 dB)."
            }
        };

        topics["rhythmic_pumper"] = {
            "Rhythmic Pumper (Tempo-Synced Sidechain Envelope)",
            "Dual-mode ducking engine locked to host PPQ tempo or triggered by external sidechain audio.",
            "Stage 5: Rhythmic Ducking & Movement",
            {
                "Amount: Modulation depth of the pumping envelope (0% to 100%).",
                "Sync Rate: Tempo division (1/1 to 1/16, dotted and triplets).",
                "Shape: Classic Duck, Hard Kick Clear, Reverse Bass, and Festival Swell.",
                "Curve: Skews envelope attack vs recovery timing.",
                "Sub Only: Confines pumping strictly to frequencies below 180 Hz."
            }
        };

        topics["maximizer_clipper"] = {
            "Maximizer Clipper (Loudness Gain & ADAA Waveshaper)",
            "Final brickwall loudness stage combining gain maximizer and zero-latency anti-aliased clipper.",
            "Stage 6: Loudness Maximization",
            {
                "Drive: Input gain driving the signal into the brickwall ceiling (0 dB to +18 dB).",
                "Ceiling: Output peak ceiling (-12 dBFS to 0 dBFS).",
                "Mode: Hard Clip (flat ceiling), Warm Tube (tanh), Hardstyle Crunch (asymmetric folding).",
                "Mix: Parallel dry/wet clipper blend."
            }
        };
    }

    std::unordered_map<juce::String, HelpTopic> topics;
};

} // namespace wvzz::ui
