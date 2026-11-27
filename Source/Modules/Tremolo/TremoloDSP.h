#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "../../Core/DSP/LfoPhaseTap.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace wvzz::modules {

/**
 * @brief Tremolo with a Classic (level) mode and a Harmonic mode that modulates the band below
 * and the band above a crossover in opposite phase. Headless and real-time safe.
 */
class TremoloDSP : public IDSPModule
{
public:
    enum Shape : int { Sine = 0, Triangle = 1, Square = 2 };
    enum Mode : int { Classic = 0, Harmonic = 1 };

    explicit TremoloDSP(juce::String prefix = "trem");
    ~TremoloDSP() override = default;

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
    /** LFO value in -1..+1 for a phase in radians. */
    [[nodiscard]] static float evaluate(double phase, int shape) noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam { nullptr };
    std::atomic<float>* rateParam   { nullptr };
    std::atomic<float>* depthParam  { nullptr };
    std::atomic<float>* shapeParam  { nullptr };
    std::atomic<float>* modeParam   { nullptr };
    std::atomic<float>* stereoParam { nullptr };
    std::atomic<float>* xoverParam  { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> depthSmoothed;

    double lfoPhase { 0.0 };

    // 2 ms one-pole smoothing of the LFO, so the square shape does not click
    float lfoSmoothCoeff { 1.0f };
    float lfoStateL { 0.0f };
    float lfoStateR { 0.0f };

    // One-pole low-pass state for the harmonic band split
    float lowStateL { 0.0f };
    float lowStateR { 0.0f };

    dsp::LfoPhaseTap lfoTap;
};

} // namespace wvzz::modules
