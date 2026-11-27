#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "../BPFilter/BPFilterDSP.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>

namespace wvzz::modules {

/**
 * @brief 3-Band (Low < 120 Hz, Mid 120 Hz - 2.5 kHz, High > 2.5 kHz) Upward & Downward Compressor.
 * Features phase-aligned LR4 crossovers, dual threshold dynamics per band, and High Sheen excitation.
 */
class MultibandOTTDSP : public IDSPModule
{
public:
    explicit MultibandOTTDSP(juce::String prefix = "ott");
    ~MultibandOTTDSP() override = default;

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

    void setParameters(float depthNorm, float upDriveNorm, float downDriveNorm,
                       float timeNorm = 1.0f, float sheenDb = 0.0f, bool bypass = false) noexcept;

private:
    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam    { nullptr };
    std::atomic<float>* depthParam     { nullptr };
    std::atomic<float>* upDriveParam   { nullptr };
    std::atomic<float>* downDriveParam { nullptr };
    std::atomic<float>* timeParam      { nullptr };
    std::atomic<float>* sheenParam     { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> depthSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> upDriveSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> downDriveSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> timeSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sheenSmoothed;

    // Crossover 1: 120 Hz Low/Mid split (Cascaded CytomicSVF pair)
    CytomicSVF xover1Lp1L, xover1Lp2L;
    CytomicSVF xover1Lp1R, xover1Lp2R;

    // Crossover 2: 2500 Hz Mid/High split (Cascaded CytomicSVF pair)
    CytomicSVF xover2Lp1L, xover2Lp2L;
    CytomicSVF xover2Lp1R, xover2Lp2R;

    // High Sheen High-Shelf filter
    CytomicSVF sheenShelfL, sheenShelfR;

    // Per-band RMS envelope states
    float envLowL  { 0.0f }, envLowR  { 0.0f };
    float envMidL  { 0.0f }, envMidR  { 0.0f };
    float envHighL { 0.0f }, envHighR { 0.0f };
};

} // namespace wvzz::modules
