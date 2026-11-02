#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <atomic>

namespace wvzz::modules {

class CompressorDSP : public IDSPModule,
                      public IMeterSource
{
public:
    explicit CompressorDSP(juce::String prefix = "comp");
    ~CompressorDSP() override = default;

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

    [[nodiscard]] float getGainReduction() const noexcept override
    {
        return gainReductionDb.load(std::memory_order_relaxed);
    }

    void setParameters(float amountNorm, float releaseMs, bool phatModeEnabled, bool bypass = false) noexcept;

private:
    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam  { nullptr };
    std::atomic<float>* amountParam  { nullptr };
    std::atomic<float>* releaseParam { nullptr };
    std::atomic<float>* phatParam    { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amountSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> releaseSmoothed;

    float envelope { 0.0f };
    std::atomic<float> gainReductionDb { 0.0f };
};

} // namespace wvzz::modules
