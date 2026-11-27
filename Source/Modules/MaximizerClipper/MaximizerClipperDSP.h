#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>

namespace wvzz::modules {

inline const juce::StringArray& getClipperModeNames()
{
    static const juce::StringArray names {
        "Hard Clip",
        "Warm Tube",
        "Hardstyle Crunch"
    };
    return names;
}

/**
 * @brief Zero-latency brickwall loudness maximizer and 1st-order ADAA anti-aliased clipper.
 */
class MaximizerClipperDSP : public IDSPModule
{
public:
    explicit MaximizerClipperDSP(juce::String prefix = "clip");
    ~MaximizerClipperDSP() override = default;

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

    void setParameters(float driveDb, float ceilingDb = -0.1f, int mode = 0,
                       float mixNorm = 1.0f, bool bypass = false) noexcept;

private:
    static inline float adaaAntiderivative(float u, int mode) noexcept
    {
        if (mode == 0) // Hard Clip: clamp(u, -1, 1)
        {
            const float absU = std::abs(u);
            if (absU <= 1.0f)
                return 0.5f * u * u;
            return absU - 0.5f;
        }
        else if (mode == 1) // Warm Tube: tanh(u)
        {
            const float absU = std::abs(u);
            if (absU > 16.0f)
                return absU - 0.69314718056f; // ln(2)
            return std::log(std::cosh(u));
        }
        else // Hardstyle Crunch: asymmetric folding inflection
        {
            const float absU = std::abs(u);
            if (absU <= 0.7f)
                return 0.5f * u * u;
            if (absU <= 1.2f)
                return 0.35f * absU + 0.15f * std::sin(absU * juce::MathConstants<float>::pi);
            return absU - 0.45f;
        }
    }

    static inline float processADAA(float u, float& prevU, int mode) noexcept
    {
        const float delta = u - prevU;
        float y = 0.0f;
        if (std::abs(delta) > 1.0e-5f)
        {
            y = (adaaAntiderivative(u, mode) - adaaAntiderivative(prevU, mode)) / delta;
        }
        else
        {
            const float mid = 0.5f * (u + prevU);
            if (mode == 0) y = juce::jlimit(-1.0f, 1.0f, mid);
            else if (mode == 1) y = std::tanh(mid);
            else y = juce::jlimit(-1.0f, 1.0f, mid > 0.0f ? (std::tanh(mid * 1.5f)) : (mid * 0.9f));
        }
        prevU = u;
        return y;
    }

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam  { nullptr };
    std::atomic<float>* driveParam   { nullptr };
    std::atomic<float>* ceilingParam { nullptr };
    std::atomic<float>* modeParam    { nullptr };
    std::atomic<float>* mixParam     { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> driveSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> ceilingSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;

    float prevInL { 0.0f };
    float prevInR { 0.0f };

    int modeFallback { 0 };
    bool bypassFallback { false };
};

} // namespace wvzz::modules
