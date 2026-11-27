#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "../../Core/DSP/LfoPhaseTap.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>

namespace wvzz::modules {

/**
 * @brief Phaser: cascade of first-order all-pass sections swept by an LFO, with feedback.
 * Headless and real-time safe. All state is fixed-size, so prepare() allocates nothing.
 */
class PhaserDSP : public IDSPModule
{
public:
    static constexpr int maxStages = 12;

    explicit PhaserDSP(juce::String prefix = "phs");
    ~PhaserDSP() override = default;

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

    /** Number of all-pass sections for a `stages` choice index (0..4 -> 2, 4, 6, 8, 12). */
    [[nodiscard]] static int stagesForIndex(int index) noexcept
    {
        static constexpr int counts[] = { 2, 4, 6, 8, 12 };
        return counts[juce::jlimit(0, 4, index)];
    }

    [[nodiscard]] const dsp::LfoPhaseTap& getLfoTap() const noexcept { return lfoTap; }

private:
    struct AllPassChain
    {
        std::array<float, maxStages> x1 {};
        std::array<float, maxStages> y1 {};
        float feedback { 0.0f };

        void clear() noexcept { x1.fill(0.0f); y1.fill(0.0f); feedback = 0.0f; }
    };

    [[nodiscard]] float processChain(AllPassChain& chain, float input, float coeff, int stages, float fb) const noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam   { nullptr };
    std::atomic<float>* rateParam     { nullptr };
    std::atomic<float>* depthParam    { nullptr };
    std::atomic<float>* centerParam   { nullptr };
    std::atomic<float>* stagesParam   { nullptr };
    std::atomic<float>* feedbackParam { nullptr };
    std::atomic<float>* stereoParam   { nullptr };
    std::atomic<float>* mixParam      { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> depthSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> centerSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> feedbackSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;

    double lfoPhase { 0.0 };
    AllPassChain chainL;
    AllPassChain chainR;

    dsp::LfoPhaseTap lfoTap;
};

} // namespace wvzz::modules
