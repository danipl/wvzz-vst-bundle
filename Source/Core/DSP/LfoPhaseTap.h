#pragma once

#include <atomic>

namespace wvzz::dsp {

/**
 * @brief Lock-free snapshot of one effect LFO, written by the audio thread once per block
 * and read by the GUI thread (LFO phase scope). Same idea as AudioVisualizerSource:
 * only atomics, no locks, no allocation. Has no GUI dependency.
 */
struct LfoPhaseTap
{
    /** Waveform ids understood by the scope. */
    enum Shape : int { Sine = 0, Triangle = 1, Square = 2 };

    std::atomic<float> phase        { 0.0f };  // 0..1 position in the LFO cycle (left channel)
    std::atomic<float> stereoOffset { 0.0f };  // 0..1 of a cycle added for the right channel
    std::atomic<float> depth        { 0.0f };  // 0..1 modulation depth, scales the drawn waveform
    std::atomic<int>   shape        { Sine };
    std::atomic<bool>  active       { false }; // false while the effect is bypassed

    void publish(float phase01, float stereoOffset01, float depth01, int shapeId, bool isActive) noexcept
    {
        phase.store(phase01, std::memory_order_relaxed);
        stereoOffset.store(stereoOffset01, std::memory_order_relaxed);
        depth.store(depth01, std::memory_order_relaxed);
        shape.store(shapeId, std::memory_order_relaxed);
        active.store(isActive, std::memory_order_relaxed);
    }

    void setActive(bool isActive) noexcept { active.store(isActive, std::memory_order_relaxed); }
};

} // namespace wvzz::dsp
