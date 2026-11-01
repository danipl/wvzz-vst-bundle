#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

namespace camelphat {

namespace ParameterIDs {
    // BP Filter
    inline constexpr auto bpCutoffLow    = "bp_cutoff_low";
    inline constexpr auto bpCutoffHigh   = "bp_cutoff_high";
    inline constexpr auto bpResonance    = "bp_resonance";
    inline constexpr auto bpMix          = "bp_mix";
    inline constexpr auto bandRejectMix  = "band_reject_mix";

    // Distortion
    inline constexpr auto distTube       = "dist_tube";
    inline constexpr auto distMech       = "dist_mech";
    inline constexpr auto distBit        = "dist_bit";
    inline constexpr auto distXcita      = "dist_xcita";
    inline constexpr auto distMix        = "dist_mix";

    // MM Filter
    inline constexpr auto mmCutoff       = "mm_cutoff";
    inline constexpr auto mmResonance    = "mm_resonance";
    inline constexpr auto mmMode         = "mm_mode";
    inline constexpr auto mmEnvAmount    = "mm_env_amount";
    inline constexpr auto mmEnvAttack    = "mm_env_attack";
    inline constexpr auto mmEnvRelease   = "mm_env_release";

    // Flanger
    inline constexpr auto flangerRate     = "flanger_rate";
    inline constexpr auto flangerDepth    = "flanger_depth";
    inline constexpr auto flangerFeedback = "flanger_feedback";
    inline constexpr auto flangerMix      = "flanger_mix";

    // Magic EQ
    inline constexpr auto magicEqAmount    = "magic_eq_amount";
    inline constexpr auto magicEqFrequency = "magic_eq_frequency";

    // Compressor
    inline constexpr auto compThreshold = "comp_threshold";
    inline constexpr auto compRatio     = "comp_ratio";
    inline constexpr auto compAttack    = "comp_attack";
    inline constexpr auto compRelease   = "comp_release";
    inline constexpr auto compPhatMode  = "comp_phat_mode";
    inline constexpr auto compMix       = "comp_mix";

    // Master
    inline constexpr auto masterMix        = "master_mix";
    inline constexpr auto masterOutputGain = "master_output_gain";
}

struct BPFilterParams
{
    std::atomic<float>* cutoffLow    { nullptr };
    std::atomic<float>* cutoffHigh   { nullptr };
    std::atomic<float>* resonance    { nullptr };
    std::atomic<float>* mix          { nullptr };
    std::atomic<float>* bandRejectMix{ nullptr };
};

struct DistortionParams
{
    std::atomic<float>* tube { nullptr };
    std::atomic<float>* mech { nullptr };
    std::atomic<float>* bit  { nullptr };
    std::atomic<float>* xcita{ nullptr };
    std::atomic<float>* mix  { nullptr };
};

struct MMFilterParams
{
    std::atomic<float>* cutoff    { nullptr };
    std::atomic<float>* resonance { nullptr };
    std::atomic<float>* mode      { nullptr }; // 0: Lowpass, 1: Bandpass, 2: Highpass
    std::atomic<float>* envAmount { nullptr };
    std::atomic<float>* envAttack { nullptr };
    std::atomic<float>* envRelease{ nullptr };
};

struct FlangerParams
{
    std::atomic<float>* rate    { nullptr };
    std::atomic<float>* depth   { nullptr };
    std::atomic<float>* feedback{ nullptr };
    std::atomic<float>* mix     { nullptr };
};

struct MagicEQParams
{
    std::atomic<float>* amount   { nullptr };
    std::atomic<float>* frequency{ nullptr };
};

struct CompressorParams
{
    std::atomic<float>* threshold{ nullptr };
    std::atomic<float>* ratio    { nullptr };
    std::atomic<float>* attack   { nullptr };
    std::atomic<float>* release  { nullptr };
    std::atomic<float>* phatMode { nullptr };
    std::atomic<float>* mix      { nullptr };
};

struct MasterParams
{
    std::atomic<float>* mix       { nullptr };
    std::atomic<float>* outputGain{ nullptr };
};

/**
 * @brief Aggregates atomic pointers to all APVTS parameters for zero-lookup audio thread performance.
 */
struct ParameterPointers
{
    BPFilterParams   bp;
    DistortionParams dist;
    MMFilterParams   mm;
    FlangerParams    flanger;
    MagicEQParams    magicEq;
    CompressorParams comp;
    MasterParams     master;

    void initialize(const juce::AudioProcessorValueTreeState& apvts)
    {
        // BP Filter
        bp.cutoffLow     = apvts.getRawParameterValue(ParameterIDs::bpCutoffLow);
        bp.cutoffHigh    = apvts.getRawParameterValue(ParameterIDs::bpCutoffHigh);
        bp.resonance     = apvts.getRawParameterValue(ParameterIDs::bpResonance);
        bp.mix           = apvts.getRawParameterValue(ParameterIDs::bpMix);
        bp.bandRejectMix = apvts.getRawParameterValue(ParameterIDs::bandRejectMix);

        // Distortion
        dist.tube  = apvts.getRawParameterValue(ParameterIDs::distTube);
        dist.mech  = apvts.getRawParameterValue(ParameterIDs::distMech);
        dist.bit   = apvts.getRawParameterValue(ParameterIDs::distBit);
        dist.xcita = apvts.getRawParameterValue(ParameterIDs::distXcita);
        dist.mix   = apvts.getRawParameterValue(ParameterIDs::distMix);

        // MM Filter
        mm.cutoff     = apvts.getRawParameterValue(ParameterIDs::mmCutoff);
        mm.resonance  = apvts.getRawParameterValue(ParameterIDs::mmResonance);
        mm.mode       = apvts.getRawParameterValue(ParameterIDs::mmMode);
        mm.envAmount  = apvts.getRawParameterValue(ParameterIDs::mmEnvAmount);
        mm.envAttack  = apvts.getRawParameterValue(ParameterIDs::mmEnvAttack);
        mm.envRelease = apvts.getRawParameterValue(ParameterIDs::mmEnvRelease);

        // Flanger
        flanger.rate     = apvts.getRawParameterValue(ParameterIDs::flangerRate);
        flanger.depth    = apvts.getRawParameterValue(ParameterIDs::flangerDepth);
        flanger.feedback = apvts.getRawParameterValue(ParameterIDs::flangerFeedback);
        flanger.mix      = apvts.getRawParameterValue(ParameterIDs::flangerMix);

        // Magic EQ
        magicEq.amount    = apvts.getRawParameterValue(ParameterIDs::magicEqAmount);
        magicEq.frequency = apvts.getRawParameterValue(ParameterIDs::magicEqFrequency);

        // Compressor
        comp.threshold = apvts.getRawParameterValue(ParameterIDs::compThreshold);
        comp.ratio     = apvts.getRawParameterValue(ParameterIDs::compRatio);
        comp.attack    = apvts.getRawParameterValue(ParameterIDs::compAttack);
        comp.release   = apvts.getRawParameterValue(ParameterIDs::compRelease);
        comp.phatMode  = apvts.getRawParameterValue(ParameterIDs::compPhatMode);
        comp.mix       = apvts.getRawParameterValue(ParameterIDs::compMix);

        // Master
        master.mix        = apvts.getRawParameterValue(ParameterIDs::masterMix);
        master.outputGain = apvts.getRawParameterValue(ParameterIDs::masterOutputGain);
    }
};

/**
 * @brief Factory function creating the full APVTS parameter layout.
 */
inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto freqSkew = juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.3f);
    auto timeSkew = juce::NormalisableRange<float>(0.1f, 500.0f, 0.1f, 0.4f);
    auto relSkew  = juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f, 0.4f);

    // BP Filter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::bpCutoffLow, 1 }, "BP Low Cut", freqSkew, 20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::bpCutoffHigh, 1 }, "BP High Cut", freqSkew, 20000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::bpResonance, 1 }, "BP Resonance", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::bpMix, 1 }, "BP Mix", 0.0f, 1.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::bandRejectMix, 1 }, "Band Reject Mix", 0.0f, 1.0f, 0.0f));

    // Distortion
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::distTube, 1 }, "Tube Drive", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::distMech, 1 }, "Mech Drive", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::distBit, 1 }, "Bit Crusher", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::distXcita, 1 }, "Xcita Drive", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::distMix, 1 }, "Distortion Mix", 0.0f, 1.0f, 1.0f));

    // MM Filter
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmCutoff, 1 }, "MM Cutoff", freqSkew, 20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmResonance, 1 }, "MM Resonance", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParameterIDs::mmMode, 1 }, "MM Filter Mode",
        juce::StringArray{ "Lowpass", "Bandpass", "Highpass" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmEnvAmount, 1 }, "MM Env Amount", -1.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmEnvAttack, 1 }, "MM Env Attack", timeSkew, 10.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmEnvRelease, 1 }, "MM Env Release", relSkew, 100.0f));

    // Flanger
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::flangerRate, 1 }, "Flanger Rate",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.01f, 0.4f), 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::flangerDepth, 1 }, "Flanger Depth", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::flangerFeedback, 1 }, "Flanger Feedback", -0.95f, 0.95f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::flangerMix, 1 }, "Flanger Mix", 0.0f, 1.0f, 0.0f));

    // Magic EQ
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::magicEqAmount, 1 }, "Magic EQ Amount", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::magicEqFrequency, 1 }, "Magic EQ Frequency", 30.0f, 120.0f, 60.0f));

    // Compressor
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::compThreshold, 1 }, "Compressor Threshold", -60.0f, 0.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::compRatio, 1 }, "Compressor Ratio", 1.0f, 20.0f, 2.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::compAttack, 1 }, "Compressor Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.4f), 10.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::compRelease, 1 }, "Compressor Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.4f), 100.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::compPhatMode, 1 }, "Phat Mode", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::compMix, 1 }, "Compressor Mix", 0.0f, 1.0f, 1.0f));

    // Master
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::masterMix, 1 }, "Master Mix", 0.0f, 1.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::masterOutputGain, 1 }, "Master Gain", -24.0f, 24.0f, 0.0f));

    return layout;
}

} // namespace camelphat
