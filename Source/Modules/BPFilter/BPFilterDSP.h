#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace wvzz::modules {

/**
 * @brief Andy Simper (Cytomic) ZDF State Variable Filter.
 */
class CytomicSVF
{
public:
    CytomicSVF() = default;

    void reset() noexcept
    {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    struct Output
    {
        float lp { 0.0f };
        float bp { 0.0f };
        float hp { 0.0f };
    };

    [[nodiscard]] inline Output processSample(float in, float g, float k) noexcept
    {
        const float denom = 1.0f / (1.0f + g * (g + k));
        const float v1 = (s1 + g * (in - s2)) * denom;
        const float v2 = s2 + g * v1;

        s1 = 2.0f * v1 - s1;
        s2 = 2.0f * v2 - s2;

        Output out;
        out.lp = v2;
        out.bp = v1;
        out.hp = in - k * v1 - v2;
        return out;
    }

private:
    float s1 { 0.0f };
    float s2 { 0.0f };
};

/**
 * @brief Headless real-time DSP implementation for BP Filter.
 */
class BPFilterDSP : public IDSPModule
{
public:
    explicit BPFilterDSP(juce::String prefix = "bp");
    ~BPFilterDSP() override = default;

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

    [[nodiscard]] float getCurrentBrMix() const noexcept { return lastModBrMix; }
    void recombine(ProcessContext& ctx) noexcept;

    // Direct parameter setters for headless testing
    void setCutoffs(float lowCutHz, float highCutHz) noexcept;
    void setResonance(float lowRes, float highRes) noexcept;
    void setBrMix(float brMix) noexcept;
    void setBypass(bool bypassed) noexcept;

private:
    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    // Raw atomic parameter pointers
    std::atomic<float>* bypassParam  { nullptr };
    std::atomic<float>* lowCutParam  { nullptr };
    std::atomic<float>* highCutParam { nullptr };
    std::atomic<float>* linkParam    { nullptr };
    std::atomic<float>* lowResParam  { nullptr };
    std::atomic<float>* highResParam { nullptr };
    std::atomic<float>* brMixParam   { nullptr };

    // Parameter smoothers (15ms ramps)
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> lowCutSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> highCutSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> lowResSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> highResSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> brMixSmoothed;

    float lastModBrMix { 0.0f };

    CytomicSVF svfLowL;
    CytomicSVF svfLowR;
    CytomicSVF svfHighL;
    CytomicSVF svfHighR;
    CytomicSVF svfHighCutInL;
    CytomicSVF svfHighCutInR;
};

} // namespace wvzz::modules
