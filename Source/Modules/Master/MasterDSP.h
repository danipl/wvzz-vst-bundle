#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <atomic>
#include <cmath>

namespace wvzz::modules {

class MasterDSP : public IDSPModule,
                  public IMeterSource
{
public:
    explicit MasterDSP(juce::String prefix = "master");
    ~MasterDSP() override = default;

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

    void setLatencyCompensation(int delaySamples) noexcept
    {
        latencySamples = juce::jlimit(0, maxDelaySamples - 1, delaySamples);
    }

    [[nodiscard]] float getPeakLevel(int channel) const noexcept override
    {
        return (channel == 0) ? peakLeft.load(std::memory_order_relaxed)
                              : peakRight.load(std::memory_order_relaxed);
    }

    [[nodiscard]] float getRmsLevel(int channel) const noexcept override
    {
        return (channel == 0) ? rmsLeft.load(std::memory_order_relaxed)
                              : rmsRight.load(std::memory_order_relaxed);
    }

    [[nodiscard]] float getInputPeakLevel(int channel) const noexcept
    {
        return (channel == 0) ? inPeakLeft.load(std::memory_order_relaxed)
                              : inPeakRight.load(std::memory_order_relaxed);
    }

    [[nodiscard]] float getInputRmsLevel(int channel) const noexcept
    {
        return (channel == 0) ? inRmsLeft.load(std::memory_order_relaxed)
                              : inRmsRight.load(std::memory_order_relaxed);
    }

    void setParameters(float inputGainDb, float mixNorm, float volDb, bool bypass = false) noexcept;

    // Helper to apply input gain before processing chain
    void applyInputGain(juce::AudioBuffer<float>& buffer) noexcept;

private:
    static inline void updateMeter(std::atomic<float>& target, float newBlockValue) noexcept
    {
        const float current = target.load(std::memory_order_relaxed);
        const float updated = (newBlockValue > current) ? newBlockValue : (current * 0.85f + newBlockValue * 0.15f);
        target.store(updated, std::memory_order_relaxed);
    }

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* inputGainParam { nullptr };
    std::atomic<float>* mixParam       { nullptr };
    std::atomic<float>* volParam       { nullptr };
    std::atomic<float>* bypassParam    { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGainSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> volSmoothed;

    int maxDelaySamples { 2048 };
    int latencySamples { 0 };
    int delayWriteIndex { 0 };

    std::vector<float> dryDelayBufferL;
    std::vector<float> dryDelayBufferR;

    std::atomic<float> peakLeft  { 0.0f };
    std::atomic<float> peakRight { 0.0f };
    std::atomic<float> rmsLeft   { 0.0f };
    std::atomic<float> rmsRight  { 0.0f };

    std::atomic<float> inPeakLeft  { 0.0f };
    std::atomic<float> inPeakRight { 0.0f };
    std::atomic<float> inRmsLeft   { 0.0f };
    std::atomic<float> inRmsRight  { 0.0f };
};

} // namespace wvzz::modules
