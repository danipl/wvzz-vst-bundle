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

        // Fallback topic
        return {
            "Module Information",
            "Contextual guidance for this component is currently active.",
            "Signal Processing Path",
            { "Use the rotary knobs to adjust parameter amounts smoothly." }
        };
    }

private:
    void initTopics()
    {
        topics["bp_filter"] = {
            "BP Filter & Band-Reject Splitter",
            "Splits the incoming audio into two separate paths: an active passband sent to the distortion and effects chain, and an untouched clean band-reject path for out-of-band audio.",
            "Stage 1: Input Splitter (Pre-Distortion)",
            {
                "Low Cut: Removes low rumble from entering the distortion stage (20 Hz - 20 kHz).",
                "High Cut: Removes harsh high frequencies before distortion (20 Hz - 20 kHz).",
                "BP Resonance: Boosts the crossover cutoff edges for resonant analog character.",
                "BP Mix: Blends between raw input and filtered passband audio.",
                "Band-Reject Mix: Injects the clean, un-distorted sub-bass and treble back into the wet signal downstream."
            }
        };

        topics["distortion"] = {
            "4-Stage Oversampled Distortion",
            "Four distinct analog and digital distortion circuits operating in 4x minimum-phase polyphase IIR oversampling with a 10 Hz DC blocking filter.",
            "Stage 2: Non-Linear Saturation (Passband Audio)",
            {
                "Tube: Smooth, warm analog overdrive with rich even-order harmonics.",
                "Mech: Aggressive, industrial hard clipping with biting metallic edge.",
                "Bit Crusher: Digital sample-rate decimation and bit-depth quantization for gritty lo-fi grit.",
                "Xcita: Dynamic high-frequency harmonic exciter that injects air and presence above 3 kHz.",
                "Dist Mix: Blends all four parallel distortion stages with the undistorted passband."
            }
        };

        topics["mm_filter"] = {
            "Multi-Mode Filter & Envelope Follower",
            "State-Variable Filter (SVF) offering Lowpass, Bandpass, and Highpass curves, dynamically modulated by incoming audio transients.",
            "Stage 3: Resonant Shaping & Dynamic Sweeps",
            {
                "Cutoff: Base frequency of the filter (20 Hz - 20 kHz).",
                "Resonance: Resonance sharpness at the cutoff point (0% - 100%).",
                "Mode: Selects between Lowpass, Bandpass, or Highpass response.",
                "Env Amount: Sweeps the cutoff dynamically up or down based on audio peak levels.",
                "Env Attack & Release: Calibrates how fast the envelope follower reacts to transients."
            }
        };

        topics["flanger"] = {
            "Stereo Modulated Flanger",
            "Short modulated stereo delay lines driven by a quadrature 90-degree LFO with soft-saturating feedback.",
            "Stage 4: Stereo Spatialization & Comb Filtering",
            {
                "Rate: Controls LFO sweep speed (0.01 Hz to 10 Hz).",
                "Depth: Controls delay sweep amplitude across the stereo channels.",
                "Feedback: Deepens the comb-filter resonance (soft-clamped with tanh to prevent runaway feedback).",
                "Flanger Mix: Blends the flanged signal with the filtered audio."
            }
        };

        topics["magic_eq"] = {
            "Magic EQ Sub-Bass Booster",
            "A specialized resonant low-frequency peaking filter with non-linear saturation designed to fatten kick drums and 808 basslines.",
            "Stage 5: Low-End Harmonic Reinforcement",
            {
                "Frequency: Centers the resonant sub boost between 30 Hz and 120 Hz (default 60 Hz).",
                "Amount: Adds up to +12 dB of resonant bass lift while gently compressing peaks with warm analog saturation."
            }
        };

        topics["compressor"] = {
            "Optical Compressor & Phat Mode",
            "Program-dependent dynamic compressor modeled after vintage optocoupler leveling amplifiers.",
            "Stage 6: Dynamics Glue & Punch",
            {
                "Threshold: Determines the volume level at which compression begins (-60 dB to 0 dB).",
                "Ratio: Sets dynamic attenuation slope from gentle 1:1 up to heavy 20:1.",
                "Attack & Release: Shapes transient punch with program-dependent release timing.",
                "Phat Mode: Engages analog transformer/tube drive in the gain stage for dense low-mid body.",
                "Comp Mix: Allows parallel 'New York style' compression."
            }
        };

        topics["master"] = {
            "Master Section & VU Meter",
            "Final output stage controlling dry/wet balance against the original input audio, master output volume, and peak/RMS metering.",
            "Stage 7: Output Mixing & Driver Safety",
            {
                "Master Mix: Crossfades between 100% dry pre-effect audio and 100% wet multi-effect audio.",
                "Output Gain: Provides -24 dB to +24 dB of clean output level makeup.",
                "VU Meters: High-speed 60 FPS animation reading atomic Peak and RMS levels with safety clamping."
            }
        };

        topics["xy_filter"] = {
            "Filter X/Y Modulation Pad",
            "Interactive 2D control pad for dynamic multi-mode filter sweeps in live performance or tracking.",
            "Interactive Gesture Control",
            {
                "Horizontal Axis (X): Controls Filter Cutoff frequency (Left = 20 Hz, Right = 20 kHz).",
                "Vertical Axis (Y): Controls Filter Resonance (Bottom = 0%, Top = 100%)."
            }
        };

        topics["xy_dist"] = {
            "Distortion X/Y Blend Pad",
            "Interactive 2D control pad balancing warm analog drive against industrial metallic bite.",
            "Interactive Gesture Control",
            {
                "Horizontal Axis (X): Controls Tube overdrive amount (0% to 100%).",
                "Vertical Axis (Y): Controls Mech harsh clipping amount (0% to 100%)."
            }
        };
    }

    std::unordered_map<juce::String, HelpTopic> topics;
};

} // namespace camelphat::ui
