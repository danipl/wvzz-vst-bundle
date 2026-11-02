#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include <juce_core/juce_core.h>
#include <array>
#include <cmath>
#include <random>

namespace wvzz::modules {

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

class ModulationDSP : public IDSPModule
{
public:
    static constexpr int numTargets = 21;

    explicit ModulationDSP(juce::String prefix = "mod");
    ~ModulationDSP() override = default;

    [[nodiscard]] juce::String paramId(const juce::String& name) const
    {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    }

    void registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    void bindParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() noexcept override;
    void process(ProcessContext& ctx) noexcept override;
    void randomize(juce::Random& rng) override;

    void setHostBpm(double bpm) noexcept { hostBpm = bpm; }

    [[nodiscard]] float getOffset(ModTarget target) const noexcept
    {
        const int idx = static_cast<int>(target);
        if (idx <= 0 || idx >= numTargets)
            return 0.0f;
        return targetOffsetsCurrent[static_cast<size_t>(idx)];
    }

    [[nodiscard]] const float* getTargetOffsets() const noexcept { return targetOffsetsCurrent.data(); }
    [[nodiscard]] int getNumTargets() const noexcept { return numTargets; }

    [[nodiscard]] float getLfo1Snapshot() const noexcept { return currentLfo1Snap.load(std::memory_order_relaxed); }
    [[nodiscard]] float getLfo2Snapshot() const noexcept { return currentLfo2Snap.load(std::memory_order_relaxed); }
    [[nodiscard]] float getLfo1Phase() const noexcept { return currentLfo1PhaseNorm.load(std::memory_order_relaxed); }
    [[nodiscard]] float getLfo2Phase() const noexcept { return currentLfo2PhaseNorm.load(std::memory_order_relaxed); }

    // Direct control update for testing
    void updateBlockControl(double bpm, int numSamples) noexcept;

private:
    [[nodiscard]] float getSyncedRateHz(int divIndex, double bpm) const noexcept;
    [[nodiscard]] float evaluateLfo(double phase, int wave, float shCurr, float shNext) noexcept;
    void advanceLfoPhase(double& phase, float rateHz, int samples, int wave, float& shCurr, float& shNext) noexcept;

    juce::String instancePrefix;
    double currentSampleRate { 44100.0 };
    double hostBpm { 120.0 };

    // Raw atomic parameter pointers
    std::atomic<float>* lfo1WaveParam    { nullptr };
    std::atomic<float>* lfo1SyncParam    { nullptr };
    std::atomic<float>* lfo1RateParam    { nullptr };
    std::atomic<float>* lfo1SyncDivParam { nullptr };
    std::atomic<float>* lfo1TargetParam  { nullptr };
    std::atomic<float>* lfo1DepthParam   { nullptr };

    std::atomic<float>* lfo2WaveParam    { nullptr };
    std::atomic<float>* lfo2SyncParam    { nullptr };
    std::atomic<float>* lfo2RateParam    { nullptr };
    std::atomic<float>* lfo2SyncDivParam { nullptr };
    std::atomic<float>* lfo2TargetParam  { nullptr };
    std::atomic<float>* lfo2DepthParam   { nullptr };

    std::atomic<float>* xyXParam       { nullptr };
    std::atomic<float>* xyYParam       { nullptr };
    std::atomic<float>* xyXTargetParam { nullptr };
    std::atomic<float>* xyYTargetParam { nullptr };
    std::atomic<float>* xyXDepthParam  { nullptr };
    std::atomic<float>* xyYDepthParam  { nullptr };

    double lfo1Phase { 0.0 };
    double lfo2Phase { 0.0 };

    float lfo1ShCurrent { 0.0f };
    float lfo1ShNext { 0.0f };
    float lfo2ShCurrent { 0.0f };
    float lfo2ShNext { 0.0f };

    std::array<float, numTargets> targetOffsetsCurrent {};
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist { -1.0f, 1.0f };

    std::atomic<float> currentLfo1Snap { 0.0f };
    std::atomic<float> currentLfo2Snap { 0.0f };
    std::atomic<float> currentLfo1PhaseNorm { 0.0f };
    std::atomic<float> currentLfo2PhaseNorm { 0.0f };
};

} // namespace wvzz::modules
