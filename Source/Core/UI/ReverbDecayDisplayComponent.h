#pragma once

#include "../../Modules/Reverb/ReverbDecayState.h"
#include "WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace wvzz::ui {

/**
 * @brief Decay-curve display: level in dB against time. It shows the pre-delay gap, the
 * early-reflections region, and one decay line each for low, mid, and high frequencies.
 *
 * Reads only the lock-free dsp::ReverbDecayState atomics. The owner repaints it from its UI timer.
 */
class ReverbDecayDisplayComponent : public juce::Component
{
public:
    explicit ReverbDecayDisplayComponent(const dsp::ReverbDecayState& stateToShow);
    ~ReverbDecayDisplayComponent() override = default;

    void paint(juce::Graphics& g) override;

    /**
     * @brief Level of the tail in dB (0 = start of the tail) at `timeSec` after the dry sound.
     * Before the pre-delay it returns -100 (silence). Pure function, so it can be unit-tested.
     * @param extraDbPerSecond extra loss from damping at the frequency of interest (0 for mid frequencies)
     */
    [[nodiscard]] static float decayDbAt(float timeSec, float predelaySec, float decaySec, float extraDbPerSecond = 0.0f) noexcept;

    /**
     * @brief Extra loss in dB per second that damping causes at `frequencyHz`.
     * Frequencies far into the damped range lose `fullDampingDbPerSecond`; near the corner they lose less.
     * @param highPass false: high damping (acts above cornerHz). true: low damping (acts below cornerHz).
     */
    [[nodiscard]] static float dampingDbPerSecond(float frequencyHz, float cornerHz, float fullDampingDbPerSecond, bool highPass) noexcept;

private:
    const dsp::ReverbDecayState& state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbDecayDisplayComponent)
};

} // namespace wvzz::ui
