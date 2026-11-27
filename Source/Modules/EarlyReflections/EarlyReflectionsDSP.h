#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "../Delay/DelayLine.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <vector>
#include <cmath>

namespace wvzz::modules {

/**
 * @brief Early reflections: a stereo pattern of 8 discrete echoes per channel, smeared by two all-pass sections.
 *
 * Reads the main buffer and writes its result to the auxiliary bus (ctx.auxBandRejectBuffer),
 * then sets ctx.hasAuxSplit. The main buffer is not changed. The Reverb module uses the auxiliary
 * bus as extra input and as part of its wet signal. Headless and real-time safe.
 */
class EarlyReflectionsDSP : public IDSPModule
{
public:
    static constexpr int numTaps = 8;
    static constexpr float minPatternMs = 16.0f;
    static constexpr float maxPatternMs = 80.0f;

    explicit EarlyReflectionsDSP(juce::String prefix = "er");
    ~EarlyReflectionsDSP() override = default;

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

    /** Pattern length in ms for the current `er_size`. */
    [[nodiscard]] float getPatternMs() const noexcept;
    [[nodiscard]] bool isActive() const noexcept;

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

    struct Channel
    {
        dsp::DelayLine line;
        std::array<AllPass, 2> diffusers;
    };

    juce::String instancePrefix;
    double sampleRate { 44100.0 };

    std::atomic<float>* bypassParam    { nullptr };
    std::atomic<float>* sizeParam      { nullptr };
    std::atomic<float>* diffusionParam { nullptr };
    std::atomic<float>* levelParam     { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> levelSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sizeSmoothed;

    std::array<Channel, 2> channels;
};

} // namespace wvzz::modules
