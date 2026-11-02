#pragma once

#include <cmath>
#include <juce_core/juce_core.h>

namespace wvzz::modules {

/**
 * @brief Precision analog-ballistics audio envelope follower.
 * Smoothly extracts audio amplitude for filter and dynamic modulation.
 */
class EnvelopeFollower
{
public:
    EnvelopeFollower() = default;

    void prepare(double sampleRate) noexcept
    {
        currentSampleRate = sampleRate;
        reset();
    }

    void reset() noexcept
    {
        envelope = 0.0f;
    }

    void setAttackRelease(float attackMs, float releaseMs) noexcept
    {
        const float safeAttack = juce::jmax(0.1f, attackMs);
        const float safeRelease = juce::jmax(1.0f, releaseMs);

        attackCoeff  = std::exp(-1.0f / (static_cast<float>(currentSampleRate) * safeAttack * 0.001f));
        releaseCoeff = std::exp(-1.0f / (static_cast<float>(currentSampleRate) * safeRelease * 0.001f));
    }

    [[nodiscard]] inline float processSample(float input) noexcept
    {
        const float inputAbs = std::abs(input);
        if (inputAbs > envelope)
            envelope = inputAbs + attackCoeff * (envelope - inputAbs);
        else
            envelope = inputAbs + releaseCoeff * (envelope - inputAbs);

        return envelope;
    }

    [[nodiscard]] float getCurrentEnvelope() const noexcept { return envelope; }

private:
    double currentSampleRate { 44100.0 };
    float attackCoeff  { 0.0f };
    float releaseCoeff { 0.0f };
    float envelope     { 0.0f };
};

} // namespace wvzz::modules
