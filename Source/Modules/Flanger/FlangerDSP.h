#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <cmath>

namespace wvzz::modules {

class FlangerDSP : public IDSPModule
{
public:
    explicit FlangerDSP(juce::String prefix = "flg");
    ~FlangerDSP() override = default;

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

    void setParameters(float rateHz, float amountNorm, bool bypass = true) noexcept;

private:
    [[nodiscard]] inline float readLagrange(const std::vector<float>& buf, int currentWrite, float delay) const noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };
    int maxDelaySamples { 0 };
    int writeIndex { 0 };

    std::atomic<float>* bypassParam { nullptr };
    std::atomic<float>* rateParam   { nullptr };
    std::atomic<float>* amountParam { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> rateSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amountSmoothed;

    double lfoPhase { 0.0 };
    float feedbackSampleL { 0.0f };
    float feedbackSampleR { 0.0f };

    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
};

} // namespace wvzz::modules
