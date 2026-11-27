#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "../../Core/DSP/LfoPhaseTap.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <cmath>

namespace wvzz::modules {

/**
 * @brief Multi-voice stereo chorus. Up to 4 modulated delay taps read one delay line per channel.
 * Headless and real-time safe: buffers are allocated in prepare(), process() never allocates or locks.
 */
class ChorusDSP : public IDSPModule
{
public:
    static constexpr int maxVoices = 4;
    static constexpr float maxSweepMs = 5.0f;

    explicit ChorusDSP(juce::String prefix = "chor");
    ~ChorusDSP() override = default;

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

    [[nodiscard]] const dsp::LfoPhaseTap& getLfoTap() const noexcept { return lfoTap; }

private:
    [[nodiscard]] float readLagrange(const std::vector<float>& buf, float delay) const noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };
    int bufferSize { 0 };
    int writeIndex { 0 };

    std::atomic<float>* bypassParam   { nullptr };
    std::atomic<float>* rateParam     { nullptr };
    std::atomic<float>* depthParam    { nullptr };
    std::atomic<float>* delayParam    { nullptr };
    std::atomic<float>* voicesParam   { nullptr };
    std::atomic<float>* spreadParam   { nullptr };
    std::atomic<float>* feedbackParam { nullptr };
    std::atomic<float>* mixParam      { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> depthSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delaySmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;

    double lfoPhase { 0.0 };
    float feedbackL { 0.0f };
    float feedbackR { 0.0f };

    std::vector<float> bufferL;
    std::vector<float> bufferR;

    dsp::LfoPhaseTap lfoTap;
};

} // namespace wvzz::modules
