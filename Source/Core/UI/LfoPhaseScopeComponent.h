#pragma once

#include "../DSP/LfoPhaseTap.h"
#include "WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace wvzz::ui {

/**
 * @brief LFO phase scope: one lane per effect LFO. Each lane draws the waveform over one cycle
 * and a marker for the current left (filled) and right (ring) LFO position.
 *
 * Reads only the lock-free dsp::LfoPhaseTap atomics; it never touches a DSP class.
 * The owner repaints it from its 60 Hz UI timer.
 */
class LfoPhaseScopeComponent : public juce::Component
{
public:
    struct Lane
    {
        juce::String name;
        juce::Colour colour;
        const dsp::LfoPhaseTap* tap { nullptr };
    };

    explicit LfoPhaseScopeComponent(std::vector<Lane> lanesToShow);
    ~LfoPhaseScopeComponent() override = default;

    void paint(juce::Graphics& g) override;

    /** Waveform value in -1..+1 at cycle position 0..1. Public so it can be unit-tested. */
    [[nodiscard]] static float waveformValue(int shape, float position01) noexcept;

private:
    std::vector<Lane> lanes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfoPhaseScopeComponent)
};

} // namespace wvzz::ui
