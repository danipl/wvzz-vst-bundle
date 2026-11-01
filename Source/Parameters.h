#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

namespace camelphat {

namespace ParameterIDs {
    // Input
    inline constexpr auto inputGain      = "input_gain";

    // BP Filter
    inline constexpr auto bpBypass       = "bp_bypass";
    inline constexpr auto bpLowCut       = "bp_low_cut";
    inline constexpr auto bpHighCut      = "bp_high_cut";
    inline constexpr auto bpLink         = "bp_link";
    inline constexpr auto bpLowRes       = "bp_low_res";
    inline constexpr auto bpHighRes      = "bp_high_res";
    inline constexpr auto bpBrMix        = "bp_br_mix";

    // Distortion
    inline constexpr auto distBypass     = "dist_bypass";
    inline constexpr auto distMech       = "dist_mech";
    inline constexpr auto distTube       = "dist_tube";
    inline constexpr auto distBit        = "dist_bit";
    inline constexpr auto distXcita      = "dist_xcita";

    // MM Filter
    inline constexpr auto mmBypass       = "mm_bypass";
    inline constexpr auto mmType         = "mm_type";
    inline constexpr auto mmCutoff       = "mm_cutoff";
    inline constexpr auto mmRes          = "mm_res";
    inline constexpr auto mmMix          = "mm_mix";
    inline constexpr auto mmEnvAtt       = "mm_env_att";
    inline constexpr auto mmEnvRel       = "mm_env_rel";
    inline constexpr auto mmEnvDepth     = "mm_env_depth";

    // Flanger
    inline constexpr auto flgBypass      = "flg_bypass";
    inline constexpr auto flgRate        = "flg_rate";
    inline constexpr auto flgAmount      = "flg_amount";

    // Magic EQ
    inline constexpr auto meqBypass      = "meq_bypass";
    inline constexpr auto meqAmount      = "meq_amount";
    inline constexpr auto meqTune        = "meq_tune";
    inline constexpr auto meqPhat        = "meq_phat";

    // Compressor
    inline constexpr auto compBypass     = "comp_bypass";
    inline constexpr auto compAmount     = "comp_amount";
    inline constexpr auto compRelease    = "comp_release";
    inline constexpr auto compPhat       = "comp_phat";

    // Master
    inline constexpr auto masterMix      = "master_mix";
    inline constexpr auto masterVol      = "master_vol";
    inline constexpr auto masterBypass   = "master_bypass";

    // Modulation: LFO 1
    inline constexpr auto lfo1Wave       = "lfo1_wave";
    inline constexpr auto lfo1Sync       = "lfo1_sync";
    inline constexpr auto lfo1Rate       = "lfo1_rate";
    inline constexpr auto lfo1SyncDiv    = "lfo1_sync_div";
    inline constexpr auto lfo1Target     = "lfo1_target";
    inline constexpr auto lfo1Depth      = "lfo1_depth";

    // Modulation: LFO 2
    inline constexpr auto lfo2Wave       = "lfo2_wave";
    inline constexpr auto lfo2Sync       = "lfo2_sync";
    inline constexpr auto lfo2Rate       = "lfo2_rate";
    inline constexpr auto lfo2SyncDiv    = "lfo2_sync_div";
    inline constexpr auto lfo2Target     = "lfo2_target";
    inline constexpr auto lfo2Depth      = "lfo2_depth";

    // Modulation: X/Y Pad
    inline constexpr auto xyX            = "xy_x";
    inline constexpr auto xyY            = "xy_y";
    inline constexpr auto xyXTarget      = "xy_x_target";
    inline constexpr auto xyYTarget      = "xy_y_target";
    inline constexpr auto xyXDepth       = "xy_x_depth";
    inline constexpr auto xyYDepth       = "xy_y_depth";
}

// 21-item Modulatable Targets Enum
enum class ModTarget : int
{
    None = 0,
    BPLowCut,
    BPHighCut,
    BPBothCuts,
    BPLowRes,
    BPHighRes,
    BPBrMix,
    DistMech,
    DistTube,
    DistBit,
    DistXcita,
    MMCutoff,
    MMRes,
    MMMix,
    FlangerRate,
    FlangerAmount,
    MagicEQAmount,
    MagicEQTune,
    CompAmount,
    MasterMix,
    LFO2Rate // LFO 1 only
};

inline const juce::StringArray& getModTargetNames()
{
    static const juce::StringArray names {
        "None",
        "BP Low Cut",
        "BP High Cut",
        "BP Both Cuts",
        "BP Low Res",
        "BP High Res",
        "BP BR Mix",
        "Dist Mech",
        "Dist Tube",
        "Dist Bit",
        "Dist Xcita",
        "MM Cutoff",
        "MM Res",
        "MM Mix",
        "Flanger Rate",
        "Flanger Amount",
        "Magic EQ Amount",
        "Magic EQ Tune",
        "Comp Amount",
        "Master Mix",
        "LFO 2 Rate"
    };
    return names;
}

inline const juce::StringArray& getLFOWaveformNames()
{
    static const juce::StringArray names {
        "Sine",
        "Triangle",
        "Saw Down",
        "Saw Up",
        "Square",
        "Sample & Hold",
        "Smooth Random"
    };
    return names;
}

inline const juce::StringArray& getLFOSyncDivNames()
{
    static const juce::StringArray names {
        "4/1", "2/1", "1/1", "1/2", "1/4", "1/8", "1/16", "1/32",
        "4/1D", "2/1D", "1/1D", "1/2D", "1/4D", "1/8D", "1/16D", "1/32D",
        "4/1T", "2/1T", "1/1T", "1/2T", "1/4T", "1/8T", "1/16T", "1/32T"
    };
    return names;
}

inline const juce::StringArray& getMMFilterTypeNames()
{
    static const juce::StringArray names {
        "LP 12dB",
        "LP 24dB",
        "BP 12dB",
        "BP 24dB",
        "HP 12dB",
        "HP 24dB",
        "Notch",
        "Peak",
        "Comb +",
        "Comb -",
        "RingMod",
        "Vowel / Formant"
    };
    return names;
}

struct BPFilterParams
{
    std::atomic<float>* bypass       { nullptr };
    std::atomic<float>* lowCut       { nullptr };
    std::atomic<float>* highCut      { nullptr };
    std::atomic<float>* link         { nullptr };
    std::atomic<float>* lowRes       { nullptr };
    std::atomic<float>* highRes      { nullptr };
    std::atomic<float>* brMix        { nullptr };
};

struct DistortionParams
{
    std::atomic<float>* bypass       { nullptr };
    std::atomic<float>* mech         { nullptr };
    std::atomic<float>* tube         { nullptr };
    std::atomic<float>* bit          { nullptr };
    std::atomic<float>* xcita        { nullptr };
};

struct MMFilterParams
{
    std::atomic<float>* bypass       { nullptr };
    std::atomic<float>* type         { nullptr };
    std::atomic<float>* cutoff       { nullptr };
    std::atomic<float>* res          { nullptr };
    std::atomic<float>* mix          { nullptr };
    std::atomic<float>* envAtt       { nullptr };
    std::atomic<float>* envRel       { nullptr };
    std::atomic<float>* envDepth     { nullptr };
};

struct FlangerParams
{
    std::atomic<float>* bypass       { nullptr };
    std::atomic<float>* rate         { nullptr };
    std::atomic<float>* amount       { nullptr };
};

struct MagicEQParams
{
    std::atomic<float>* bypass       { nullptr };
    std::atomic<float>* amount       { nullptr };
    std::atomic<float>* tune         { nullptr };
    std::atomic<float>* phat         { nullptr };
};

struct CompressorParams
{
    std::atomic<float>* bypass       { nullptr };
    std::atomic<float>* amount       { nullptr };
    std::atomic<float>* release      { nullptr };
    std::atomic<float>* phat         { nullptr };
};

struct MasterParams
{
    std::atomic<float>* inputGain    { nullptr };
    std::atomic<float>* mix          { nullptr };
    std::atomic<float>* vol          { nullptr };
    std::atomic<float>* bypass       { nullptr };
};

struct ModulationParams
{
    std::atomic<float>* lfo1Wave     { nullptr };
    std::atomic<float>* lfo1Sync     { nullptr };
    std::atomic<float>* lfo1Rate     { nullptr };
    std::atomic<float>* lfo1SyncDiv  { nullptr };
    std::atomic<float>* lfo1Target   { nullptr };
    std::atomic<float>* lfo1Depth    { nullptr };

    std::atomic<float>* lfo2Wave     { nullptr };
    std::atomic<float>* lfo2Sync     { nullptr };
    std::atomic<float>* lfo2Rate     { nullptr };
    std::atomic<float>* lfo2SyncDiv  { nullptr };
    std::atomic<float>* lfo2Target   { nullptr };
    std::atomic<float>* lfo2Depth    { nullptr };

    std::atomic<float>* xyX          { nullptr };
    std::atomic<float>* xyY          { nullptr };
    std::atomic<float>* xyXTarget    { nullptr };
    std::atomic<float>* xyYTarget    { nullptr };
    std::atomic<float>* xyXDepth     { nullptr };
    std::atomic<float>* xyYDepth     { nullptr };
};

struct ParameterPointers
{
    MasterParams     master;
    BPFilterParams   bp;
    DistortionParams dist;
    MMFilterParams   mm;
    FlangerParams    flanger;
    MagicEQParams    magicEq;
    CompressorParams comp;
    ModulationParams mod;

    void initialize(const juce::AudioProcessorValueTreeState& apvts)
    {
        master.inputGain = apvts.getRawParameterValue(ParameterIDs::inputGain);
        master.mix       = apvts.getRawParameterValue(ParameterIDs::masterMix);
        master.vol       = apvts.getRawParameterValue(ParameterIDs::masterVol);
        master.bypass    = apvts.getRawParameterValue(ParameterIDs::masterBypass);

        bp.bypass        = apvts.getRawParameterValue(ParameterIDs::bpBypass);
        bp.lowCut        = apvts.getRawParameterValue(ParameterIDs::bpLowCut);
        bp.highCut       = apvts.getRawParameterValue(ParameterIDs::bpHighCut);
        bp.link          = apvts.getRawParameterValue(ParameterIDs::bpLink);
        bp.lowRes        = apvts.getRawParameterValue(ParameterIDs::bpLowRes);
        bp.highRes       = apvts.getRawParameterValue(ParameterIDs::bpHighRes);
        bp.brMix         = apvts.getRawParameterValue(ParameterIDs::bpBrMix);

        dist.bypass      = apvts.getRawParameterValue(ParameterIDs::distBypass);
        dist.mech        = apvts.getRawParameterValue(ParameterIDs::distMech);
        dist.tube        = apvts.getRawParameterValue(ParameterIDs::distTube);
        dist.bit         = apvts.getRawParameterValue(ParameterIDs::distBit);
        dist.xcita       = apvts.getRawParameterValue(ParameterIDs::distXcita);

        mm.bypass        = apvts.getRawParameterValue(ParameterIDs::mmBypass);
        mm.type          = apvts.getRawParameterValue(ParameterIDs::mmType);
        mm.cutoff        = apvts.getRawParameterValue(ParameterIDs::mmCutoff);
        mm.res           = apvts.getRawParameterValue(ParameterIDs::mmRes);
        mm.mix           = apvts.getRawParameterValue(ParameterIDs::mmMix);
        mm.envAtt        = apvts.getRawParameterValue(ParameterIDs::mmEnvAtt);
        mm.envRel        = apvts.getRawParameterValue(ParameterIDs::mmEnvRel);
        mm.envDepth      = apvts.getRawParameterValue(ParameterIDs::mmEnvDepth);

        flanger.bypass   = apvts.getRawParameterValue(ParameterIDs::flgBypass);
        flanger.rate     = apvts.getRawParameterValue(ParameterIDs::flgRate);
        flanger.amount   = apvts.getRawParameterValue(ParameterIDs::flgAmount);

        magicEq.bypass   = apvts.getRawParameterValue(ParameterIDs::meqBypass);
        magicEq.amount   = apvts.getRawParameterValue(ParameterIDs::meqAmount);
        magicEq.tune     = apvts.getRawParameterValue(ParameterIDs::meqTune);
        magicEq.phat     = apvts.getRawParameterValue(ParameterIDs::meqPhat);

        comp.bypass      = apvts.getRawParameterValue(ParameterIDs::compBypass);
        comp.amount      = apvts.getRawParameterValue(ParameterIDs::compAmount);
        comp.release     = apvts.getRawParameterValue(ParameterIDs::compRelease);
        comp.phat        = apvts.getRawParameterValue(ParameterIDs::compPhat);

        mod.lfo1Wave     = apvts.getRawParameterValue(ParameterIDs::lfo1Wave);
        mod.lfo1Sync     = apvts.getRawParameterValue(ParameterIDs::lfo1Sync);
        mod.lfo1Rate     = apvts.getRawParameterValue(ParameterIDs::lfo1Rate);
        mod.lfo1SyncDiv  = apvts.getRawParameterValue(ParameterIDs::lfo1SyncDiv);
        mod.lfo1Target   = apvts.getRawParameterValue(ParameterIDs::lfo1Target);
        mod.lfo1Depth    = apvts.getRawParameterValue(ParameterIDs::lfo1Depth);

        mod.lfo2Wave     = apvts.getRawParameterValue(ParameterIDs::lfo2Wave);
        mod.lfo2Sync     = apvts.getRawParameterValue(ParameterIDs::lfo2Sync);
        mod.lfo2Rate     = apvts.getRawParameterValue(ParameterIDs::lfo2Rate);
        mod.lfo2SyncDiv  = apvts.getRawParameterValue(ParameterIDs::lfo2SyncDiv);
        mod.lfo2Target   = apvts.getRawParameterValue(ParameterIDs::lfo2Target);
        mod.lfo2Depth    = apvts.getRawParameterValue(ParameterIDs::lfo2Depth);

        mod.xyX          = apvts.getRawParameterValue(ParameterIDs::xyX);
        mod.xyY          = apvts.getRawParameterValue(ParameterIDs::xyY);
        mod.xyXTarget    = apvts.getRawParameterValue(ParameterIDs::xyXTarget);
        mod.xyYTarget    = apvts.getRawParameterValue(ParameterIDs::xyYTarget);
        mod.xyXDepth     = apvts.getRawParameterValue(ParameterIDs::xyXDepth);
        mod.xyYDepth     = apvts.getRawParameterValue(ParameterIDs::xyYDepth);
    }
};

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto freqSkew = juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.25f);
    auto timeAttSkew = juce::NormalisableRange<float>(0.1f, 1000.0f, 0.1f, 0.35f);
    auto timeRelSkew = juce::NormalisableRange<float>(1.0f, 5000.0f, 1.0f, 0.35f);
    auto compRelSkew = juce::NormalisableRange<float>(10.0f, 1500.0f, 1.0f, 0.35f);
    auto lfoRateSkew = juce::NormalisableRange<float>(0.02f, 30.0f, 0.01f, 0.35f);
    auto flgRateSkew = juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f, 0.35f);

    // Master / Input
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::inputGain, 1 }, "Input Gain", -24.0f, 24.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::masterMix, 1 }, "Master Mix", 0.0f, 1.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::masterVol, 1 }, "Master Volume", -36.0f, 12.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::masterBypass, 1 }, "Master Bypass", false));

    // BP Filter
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::bpBypass, 1 }, "BP Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::bpLowCut, 1 }, "BP Low Cut", freqSkew, 20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::bpHighCut, 1 }, "BP High Cut", freqSkew, 20000.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::bpLink, 1 }, "BP Link", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::bpLowRes, 1 }, "BP Low Res", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::bpHighRes, 1 }, "BP High Res", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::bpBrMix, 1 }, "BP BR Mix", 0.0f, 1.0f, 0.0f));

    // Distortion
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::distBypass, 1 }, "Dist Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::distMech, 1 }, "Mech", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::distTube, 1 }, "Tube", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::distBit, 1 }, "Bitcrusher", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::distXcita, 1 }, "Xcita", 0.0f, 1.0f, 0.0f));

    // MM Filter
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::mmBypass, 1 }, "MM Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParameterIDs::mmType, 1 }, "MM Filter Type", getMMFilterTypeNames(), 1)); // Default 1: LP 24dB
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmCutoff, 1 }, "MM Cutoff", freqSkew, 1000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmRes, 1 }, "MM Resonance", 0.0f, 1.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmMix, 1 }, "MM Mix", 0.0f, 1.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmEnvAtt, 1 }, "MM Env Attack", timeAttSkew, 10.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmEnvRel, 1 }, "MM Env Release", timeRelSkew, 150.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::mmEnvDepth, 1 }, "MM Env Mod", -1.0f, 1.0f, 0.0f));

    // Flanger
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::flgBypass, 1 }, "Flanger Bypass", true)); // Default 1 (bypassed)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::flgRate, 1 }, "Flanger Rate", flgRateSkew, 0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::flgAmount, 1 }, "Flanger Amount", 0.0f, 1.0f, 0.0f));

    // Magic EQ
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::meqBypass, 1 }, "Magic EQ Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::meqAmount, 1 }, "Magic EQ Amount", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::meqTune, 1 }, "Magic EQ Tune", 60.0f, 120.0f, 80.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::meqPhat, 1 }, "Magic EQ Phat Mode", false));

    // Compressor
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::compBypass, 1 }, "Compressor Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::compAmount, 1 }, "Compressor Amount", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::compRelease, 1 }, "Compressor Release", compRelSkew, 120.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::compPhat, 1 }, "Compressor Phat Mode", false));

    // Modulation: LFO 1
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParameterIDs::lfo1Wave, 1 }, "LFO 1 Wave", getLFOWaveformNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::lfo1Sync, 1 }, "LFO 1 Sync", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::lfo1Rate, 1 }, "LFO 1 Rate", lfoRateSkew, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParameterIDs::lfo1SyncDiv, 1 }, "LFO 1 Sync Div", getLFOSyncDivNames(), 4)); // 1/4 default
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParameterIDs::lfo1Target, 1 }, "LFO 1 Target", getModTargetNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::lfo1Depth, 1 }, "LFO 1 Depth", -1.0f, 1.0f, 0.0f));

    // Modulation: LFO 2
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParameterIDs::lfo2Wave, 1 }, "LFO 2 Wave", getLFOWaveformNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParameterIDs::lfo2Sync, 1 }, "LFO 2 Sync", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::lfo2Rate, 1 }, "LFO 2 Rate", lfoRateSkew, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParameterIDs::lfo2SyncDiv, 1 }, "LFO 2 Sync Div", getLFOSyncDivNames(), 3)); // 1/2 default
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParameterIDs::lfo2Target, 1 }, "LFO 2 Target", getModTargetNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::lfo2Depth, 1 }, "LFO 2 Depth", -1.0f, 1.0f, 0.0f));

    // Modulation: X/Y Pad
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::xyX, 1 }, "X/Y X", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::xyY, 1 }, "X/Y Y", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParameterIDs::xyXTarget, 1 }, "X/Y X Target", getModTargetNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParameterIDs::xyYTarget, 1 }, "X/Y Y Target", getModTargetNames(), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::xyXDepth, 1 }, "X/Y X Depth", -1.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParameterIDs::xyYDepth, 1 }, "X/Y Y Depth", -1.0f, 1.0f, 0.0f));

    return layout;
}

} // namespace camelphat
