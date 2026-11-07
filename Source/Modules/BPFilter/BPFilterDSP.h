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
        s1 = 0.0;
        s2 = 0.0;
    }

    struct Output
    {
        float lp { 0.0f };
        float bp { 0.0f };
        float hp { 0.0f };
    };

    [[nodiscard]] inline Output processSample(float in, float g, float k) noexcept
    {
        const double gd = static_cast<double>(g);
        const double kd = static_cast<double>(k);
        const double ind = static_cast<double>(in);

        const double a1 = 1.0 / (1.0 + gd * (gd + kd));
        const double a2 = gd * a1;
        const double a3 = gd * a2;

        const double v3 = ind - s2;
        const double v1 = a1 * s1 + a2 * v3;
        const double v2 = s2 + a2 * s1 + a3 * v3;

        s1 = 2.0 * v1 - s1;
        s2 = 2.0 * v2 - s2;

        Output out;
        out.lp = static_cast<float>(v2);
        out.bp = static_cast<float>(v1);
        out.hp = static_cast<float>(ind - kd * v1 - v2);
        return out;
    }

private:
    double s1 { 0.0 };
    double s2 { 0.0 };
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

    static constexpr int maxRejDelaySamples = 256;
    std::array<float, maxRejDelaySamples> rejDelayBufferL {};
    std::array<float, maxRejDelaySamples> rejDelayBufferR {};
    int rejDelayWriteIndex { 0 };
};

} // namespace wvzz::modules
