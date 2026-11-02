#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "../BPFilter/BPFilterDSP.h" // For CytomicSVF
#include "EnvelopeFollower.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <array>

namespace wvzz::modules {

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

class MMFilterDSP : public IDSPModule
{
public:
    explicit MMFilterDSP(juce::String prefix = "mm");
    ~MMFilterDSP() override = default;

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

    // Direct setters for testing
    void setParameters(float cutoffHz, float resonanceNorm, int typeIndex,
                       float mixNorm, float envAttMs, float envRelMs,
                       float envDepthNorm, bool bypass = false) noexcept;

private:
    [[nodiscard]] inline float readLagrange(const std::vector<float>& buf, int currentWrite, float delaySamples) const noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam   { nullptr };
    std::atomic<float>* typeParam     { nullptr };
    std::atomic<float>* cutoffParam   { nullptr };
    std::atomic<float>* resParam      { nullptr };
    std::atomic<float>* mixParam      { nullptr };
    std::atomic<float>* envAttParam   { nullptr };
    std::atomic<float>* envRelParam   { nullptr };
    std::atomic<float>* envDepthParam { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> cutoffSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> resSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> envDepthSmoothed;

    EnvelopeFollower envFollower;

    CytomicSVF svf1L, svf1R;
    CytomicSVF svf2L, svf2R;

    CytomicSVF vowelSvf1L, vowelSvf1R;
    CytomicSVF vowelSvf2L, vowelSvf2R;
    CytomicSVF vowelSvf3L, vowelSvf3R;

    int maxCombDelaySamples { 0 };
    int combWriteIndex { 0 };
    std::vector<float> combBufferL;
    std::vector<float> combBufferR;

    double ringPhase { 0.0 };
    float ringFeedbackSampleL { 0.0f };
    float ringFeedbackSampleR { 0.0f };
};

} // namespace wvzz::modules
