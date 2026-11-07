#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "../BPFilter/BPFilterDSP.h"
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>

namespace wvzz::modules {

/**
 * @brief Sub-bass Linkwitz-Riley 24 dB/oct crossover & sidechain detector filter.
 * Isolates frequencies below subXover into auxBandRejectBuffer, applies Sub Tightening,
 * and shapes the sidechain detector path (HPF + Tilt).
 */
class DynDetectorDSP : public IDSPModule
{
public:
    explicit DynDetectorDSP(juce::String prefix = "det");
    ~DynDetectorDSP() override = default;

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

    void setParameters(float subXoverHz, bool subLock, float subTightNorm,
                       float scHpHz, float scTiltDb, bool scListen = false, bool bypass = false) noexcept;

    [[nodiscard]] bool isSubLocked() const noexcept
    {
        return (subLockParam != nullptr) ? (subLockParam->load(std::memory_order_relaxed) > 0.5f) : subLockFallback;
    }

    [[nodiscard]] bool isSidechainListen() const noexcept
    {
        return (scListenParam != nullptr) ? (scListenParam->load(std::memory_order_relaxed) > 0.5f) : scListenFallback;
    }

private:
    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam   { nullptr };
    std::atomic<float>* subXoverParam { nullptr };
    std::atomic<float>* subLockParam  { nullptr };
    std::atomic<float>* subTightParam { nullptr };
    std::atomic<float>* scHpParam     { nullptr };
    std::atomic<float>* scTiltParam   { nullptr };
    std::atomic<float>* scListenParam { nullptr };

    bool subLockFallback { true };
    bool scListenFallback { false };
    bool bypassFallback { false };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> subXoverSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> subTightSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> scHpSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> scTiltSmoothed;

    // Sub Crossover: 2 cascaded 2-pole CytomicSVFs = 4-pole Linkwitz-Riley (24 dB/oct)
    CytomicSVF subLp1L, subLp2L;
    CytomicSVF subLp1R, subLp2R;

    // Sidechain Detector Filter: 2-pole HPF + 1-pole Tilt Shelf
    CytomicSVF scHpfL, scHpfR;
    float scTiltStateL { 0.0f };
    float scTiltStateR { 0.0f };

    // Sub tightener RMS detector state
    float subEnvL { 0.0f };
    float subEnvR { 0.0f };
};

} // namespace wvzz::modules
