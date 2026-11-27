#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "../../Core/DSP/LfoPhaseTap.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <cmath>

namespace wvzz::modules {

class FlangerDSP : public IDSPModule
{
public:
    // Values that were fixed constants before flg_feedback / flg_depth / flg_center existed.
    static constexpr float defaultFeedback = 0.55f;
    static constexpr float defaultDepthMs  = 2.0f;
    static constexpr float defaultCenterMs = 2.5f;

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

    /** Lock-free LFO state for the GUI phase scope. */
    [[nodiscard]] const dsp::LfoPhaseTap& getLfoTap() const noexcept { return lfoTap; }

private:
    [[nodiscard]] inline float readLagrange(const std::vector<float>& buf, int currentWrite, float delay) const noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };
    int maxDelaySamples { 0 };
    int writeIndex { 0 };

    std::atomic<float>* bypassParam { nullptr };
    std::atomic<float>* rateParam   { nullptr };
    std::atomic<float>* amountParam { nullptr };
    std::atomic<float>* feedbackParam { nullptr };
    std::atomic<float>* depthParam    { nullptr };
    std::atomic<float>* centerParam   { nullptr };
    std::atomic<float>* tzeroParam    { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> rateSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amountSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> feedbackSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> depthSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> centerSmoothed;

    double lfoPhase { 0.0 };
    float feedbackSampleL { 0.0f };
    float feedbackSampleR { 0.0f };

    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;

    // Dry-only delay lines, read at the center delay in through-zero mode
    std::vector<float> dryBufferL;
    std::vector<float> dryBufferR;

    dsp::LfoPhaseTap lfoTap;
};

} // namespace wvzz::modules
