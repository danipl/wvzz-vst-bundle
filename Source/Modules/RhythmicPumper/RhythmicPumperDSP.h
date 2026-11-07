#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "../BPFilter/BPFilterDSP.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>

namespace wvzz::modules {

inline const juce::StringArray& getPumpSyncDivNames()
{
    static const juce::StringArray names {
        "1/1", "1/2", "1/4", "1/8", "1/16",
        "1/4D", "1/8D", "1/4T", "1/8T"
    };
    return names;
}

inline const juce::StringArray& getPumpShapeNames()
{
    static const juce::StringArray names {
        "Classic Duck",
        "Hard Kick Clear",
        "Reverse Bass",
        "Festival Swell"
    };
    return names;
}

/**
 * @brief Rhythmic Ducker with Host PPQ Tempo Sync and Audio/External Sidechain triggering.
 * Supports sub-only frequency ducking (<180 Hz) and 4 electronic music contour shapes.
 */
class RhythmicPumperDSP : public IDSPModule
{
public:
    explicit RhythmicPumperDSP(juce::String prefix = "pump");
    ~RhythmicPumperDSP() override = default;

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

    void setParameters(float amountNorm, int syncDiv = 2, int shape = 0, float curveNorm = 0.5f,
                       int mode = 0, bool subOnly = false, bool bypass = false) noexcept;

    [[nodiscard]] float getPumpGain() const noexcept
    {
        return currentPumpGain.load(std::memory_order_relaxed);
    }

private:
    [[nodiscard]] double getSyncDivBeats(int divIdx) const noexcept;
    [[nodiscard]] float evaluatePumpCurve(float phaseNorm, int shape, float curve) const noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam  { nullptr };
    std::atomic<float>* amountParam  { nullptr };
    std::atomic<float>* syncDivParam { nullptr };
    std::atomic<float>* shapeParam   { nullptr };
    std::atomic<float>* curveParam   { nullptr };
    std::atomic<float>* modeParam    { nullptr }; // 0: Tempo Sync, 1: Audio / Ext SC
    std::atomic<float>* subOnlyParam { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amountSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> curveSmoothed;

    // Sub-only crossover (180 Hz)
    CytomicSVF subSvfL, subSvfR;

    // Audio trigger tracking state
    double freePhase { 0.0 };
    float prevScPeak { 0.0f };
    std::atomic<float> currentPumpGain { 1.0f };
};

} // namespace wvzz::modules
