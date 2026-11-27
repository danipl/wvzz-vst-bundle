#pragma once

#include "../../Modules/Delay/DelayTapState.h"
#include "WvZzLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace wvzz::ui {

/**
 * @brief Delay-tap display: a time axis with one marker per repeat, left channel above the
 * centre line and right channel below. Marker height follows the feedback decay.
 *
 * Reads only the lock-free dsp::DelayTapState atomics. The owner repaints it from its UI timer.
 */
class DelayTapDisplayComponent : public juce::Component
{
public:
    static constexpr int maxTaps = 16;

    struct Tap
    {
        float timeMs { 0.0f };
        float level { 0.0f };   // 0..1
        bool rightChannel { false };
    };

    explicit DelayTapDisplayComponent(const dsp::DelayTapState& stateToShow);
    ~DelayTapDisplayComponent() override = default;

    void paint(juce::Graphics& g) override;

    /**
     * @brief Computes the repeat markers for the given settings. Pure function, so it can be unit-tested.
     * Stereo: both channels repeat at their own time. Ping-pong: repeats alternate left, right, left...
     */
    [[nodiscard]] static std::vector<Tap> computeTaps(float timeLeftMs, float timeRightMs, float feedback, bool pingPong);

private:
    const dsp::DelayTapState& state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayTapDisplayComponent)
};

} // namespace wvzz::ui
