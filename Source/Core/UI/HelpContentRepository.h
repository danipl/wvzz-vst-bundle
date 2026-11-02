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
            "Dual modulated comb filter with 90-degree stereo quadrature phase offset and bipolar feedback.",
            "Stage 4: Stereo Space & Movement",
            {
                "Rate: Modulation frequency (0.05 to 5.0 Hz).",
                "Amount: Modulation depth and wet/dry mix (0.0 to 1.0).",
                "Feedback: Bipolar feedback producing rich metallic jet flanging.",
                "Bypass: Completely bypasses the flanger stage."
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
            "Smooth RMS-based compressor with soft-knee optical release ballistics and automatic makeup gain.",
            "Stage 6: Dynamics & Glue",
            {
                "Amount: Compression intensity adjusting threshold and ratio simultaneously.",
                "Release: Optical recovery time (50 ms to 2000 ms).",
                "Phat: Engages program-dependent dynamic punch saturation."
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
    }

    std::unordered_map<juce::String, HelpTopic> topics;
};

} // namespace wvzz::ui
