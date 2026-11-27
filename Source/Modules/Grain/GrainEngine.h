#pragma once

#include "../Delay/DelayLine.h"
#include <array>
#include <cstdint>

namespace wvzz::dsp {

/**
 * @brief Granular reader with 32 fixed voices.
 *
 * It replays audio from a stereo pair of DelayLines as short Hann-windowed pieces (grains).
 * Each grain can be pitch-shifted, reversed, and placed in the stereo field.
 * No allocation: the voices live in a fixed std::array. No GUI or plugin-specific code.
 */
class GrainEngine
{
public:
    static constexpr int maxVoices = 32;

    struct Settings
    {
        float sizeSamples      { 3840.0f };  // grain length
        float grainsPerSecond  { 20.0f };
        float pitchRatio       { 1.0f };     // 2.0 = one octave up
        float spraySamples     { 0.0f };     // random extra delay per grain, 0..spraySamples
        float reverseChance    { 0.0f };     // 0..1
        float stereoSpread     { 0.0f };     // 0..1 random pan per grain
    };

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;

    /**
     * @brief Renders one stereo sample of grains.
     * @param baseDelayL / baseDelayR  delay (in samples) at which new grains start reading.
     * Call it before pushing the current input sample into the lines.
     */
    void process(const DelayLine& lineL, const DelayLine& lineR,
                 float baseDelayL, float baseDelayR,
                 const Settings& settings, float& outL, float& outR) noexcept;

    [[nodiscard]] int getNumActiveVoices() const noexcept { return activeVoices; }

private:
    struct Voice
    {
        bool active { false };
        float delayL { 0.0f };      // current read delay in samples
        float delayR { 0.0f };
        float delayStep { 0.0f };   // change of the delay per sample (0 = original pitch)
        float age { 0.0f };         // samples since the grain started
        float length { 1.0f };
        float gainL { 1.0f };
        float gainR { 1.0f };
    };

    void spawn(const DelayLine& lineL, float baseDelayL, float baseDelayR, const Settings& settings) noexcept;
    [[nodiscard]] float nextRandom() noexcept; // 0..1

    std::array<Voice, maxVoices> voices {};
    double sampleRate { 48000.0 };
    float samplesUntilNextGrain { 0.0f };
    int activeVoices { 0 };
    std::uint32_t rngState { 0x9E3779B9u };
};

} // namespace wvzz::dsp
