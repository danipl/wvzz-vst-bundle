#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "../Delay/DelayLine.h"
#include "../Grain/GrainEngine.h"
#include "ReverbDecayState.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <vector>
#include <cmath>

namespace wvzz::modules {

inline const juce::StringArray& getReverbModeNames()
{
    static const juce::StringArray names { "Plate", "Hall", "Shimmer" };
    return names;
}

inline const juce::StringArray& getShimmerIntervalNames()
{
    static const juce::StringArray names { "+1 Octave", "+Fifth", "+Octave+Fifth", "+2 Octaves", "-1 Octave" };
    return names;
}

/**
 * @brief Reverb built on one Feedback Delay Network: 8 delay lines that feed each other through
 * an energy-preserving Hadamard matrix. Modes: Plate, Hall, Shimmer (pitch-shifted tail in the loop).
 * Headless and real-time safe: every buffer is allocated in prepare().
 */
class ReverbDSP : public IDSPModule
{
public:
    static constexpr int numLines = 8;
    enum Mode : int { Plate = 0, Hall = 1, Shimmer = 2 };

    static constexpr float minDecaySeconds = 0.2f;
    static constexpr float maxDecaySeconds = 20.0f;

    /** Fully damped frequencies (far above High Damp or far below Low Damp) decay in this share of the Decay time. */
    static constexpr float dampedDecayRatio = 0.35f;

    /** Extra loss in dB per second for fully damped frequencies at a given decay time. */
    [[nodiscard]] static float fullDampingDbPerSecond(float decaySeconds) noexcept
    {
        return 60.0f / (dampedDecayRatio * decaySeconds) - 60.0f / decaySeconds;
    }

    explicit ReverbDSP(juce::String prefix = "rev");
    ~ReverbDSP() override = default;

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

    /** 8x8 Hadamard transform scaled by 1/sqrt(8). Orthogonal: it preserves the sum of squares. */
    static void mixMatrix(std::array<float, numLines>& v) noexcept;

    /** Pitch shift in semitones for a `rev_interval` choice index. */
    [[nodiscard]] static float semitonesForInterval(int index) noexcept;

    /** Base line length in ms for a mode and line index, before `rev_size` scaling. */
    [[nodiscard]] static float baseLineMs(int mode, int line) noexcept;

    /** Pre-delay + decay time from the current parameters, in seconds. */
    [[nodiscard]] double getTailSeconds() const noexcept;

    [[nodiscard]] const dsp::ReverbDecayState& getDecayState() const noexcept { return decayState; }

    /** Length of the early-reflection pattern feeding this reverb (0 = none). Shown by the decay display. */
    void setEarlyPatternMs(float ms) noexcept { decayState.earlyMs.store(ms, std::memory_order_relaxed); }

private:
    struct AllPass
    {
        std::vector<float> buffer;
        int index { 0 };
        void prepare(int lengthSamples) { buffer.assign(static_cast<size_t>(juce::jmax(1, lengthSamples)), 0.0f); index = 0; }
        void clear() noexcept { std::fill(buffer.begin(), buffer.end(), 0.0f); index = 0; }
        float process(float x, float g) noexcept
        {
            const float delayed = buffer[static_cast<size_t>(index)];
            float w = x + g * delayed;
            if (std::abs(w) < 1.0e-15f)
                w = 0.0f;
            buffer[static_cast<size_t>(index)] = w;
            if (++index >= static_cast<int>(buffer.size()))
                index = 0;
            return delayed - g * w;
        }
    };

    void clearNetwork() noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam    { nullptr };
    std::atomic<float>* modeParam      { nullptr };
    std::atomic<float>* predelayParam  { nullptr };
    std::atomic<float>* sizeParam      { nullptr };
    std::atomic<float>* decayParam     { nullptr };
    std::atomic<float>* lodampParam    { nullptr };
    std::atomic<float>* hidampParam    { nullptr };
    std::atomic<float>* diffusionParam { nullptr };
    std::atomic<float>* modParam       { nullptr };
    std::atomic<float>* widthParam     { nullptr };
    std::atomic<float>* shimmerParam   { nullptr };
    std::atomic<float>* intervalParam  { nullptr };
    std::atomic<float>* mixParam       { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> widthSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sendSmoothed;       // 1 = feed the network, 0 = bypassed
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> shimmerSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> modeFadeSmoothed;   // wet fade around a mode change

    // Pre-delay and input diffusion (per channel)
    std::array<dsp::DelayLine, 2> predelayLines;
    std::array<float, 2> predelayGlide { 0.0f, 0.0f };
    std::array<std::array<AllPass, 4>, 2> inputDiffusers;

    // Feedback Delay Network
    std::array<dsp::DelayLine, numLines> lines;
    std::array<float, numLines> lineLength {};   // current length in samples (glides on size changes)
    std::array<float, numLines> lpState {};
    std::array<float, numLines> hpState {};
    std::array<double, numLines> modPhase {};

    // Shimmer: network output -> delay lines -> grain engine (pitch up) -> back into the network
    std::array<dsp::DelayLine, 2> shimmerLines;
    dsp::GrainEngine shimmerEngine;
    std::array<float, 2> shimmerLp { 0.0f, 0.0f };

    int activeMode { Hall };
    int pendingMode { Hall };

    dsp::ReverbDecayState decayState;
};

} // namespace wvzz::modules
