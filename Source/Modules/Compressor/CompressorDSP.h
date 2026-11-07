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

    [[nodiscard]] float getGainReductionDb() const noexcept
    {
        return getGainReduction();
    }

    [[nodiscard]] bool isSaturating() const noexcept
    {
        return saturationActivity.load(std::memory_order_relaxed) > 0.5f;
    }

    void setParameters(float amountNorm, float releaseMs, bool phatModeEnabled, bool bypass = false,
                       float threshDb = -18.0f, float ratioVal = 4.0f, float attackMsVal = 5.0f, float mixVal = 1.0f) noexcept;

private:
    static inline float adaaAntiderivative(float x) noexcept
    {
        const float absX = std::abs(x);
        if (absX > 16.0f)
            return absX - 0.69314718056f; // ln(2)
        return std::log(std::cosh(x));
    }

    static inline float processADAA(float x, float& prevX) noexcept
    {
        const float delta = x - prevX;
        float y = 0.0f;
        if (std::abs(delta) > 1.0e-5f)
        {
            y = (adaaAntiderivative(x) - adaaAntiderivative(prevX)) / delta;
        }
        else
        {
            y = std::tanh(0.5f * (x + prevX));
        }
        prevX = x;
        return y;
    }

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam  { nullptr };
    std::atomic<float>* amountParam  { nullptr };
    std::atomic<float>* releaseParam { nullptr };
    std::atomic<float>* phatParam    { nullptr };
    std::atomic<float>* threshParam  { nullptr };
    std::atomic<float>* ratioParam   { nullptr };
    std::atomic<float>* attackParam  { nullptr };
    std::atomic<float>* mixParam     { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amountSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> releaseSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> threshSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> ratioSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> attackSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;

    float fastEnvelopeDb { 0.0f };
    float slowEnvelopeDb { 0.0f };
    float prevInNormL    { 0.0f };
    float prevInNormR    { 0.0f };
    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> saturationActivity { 0.0f };
};

} // namespace wvzz::modules
