#pragma once

#include <atomic>

namespace wvzz::dsp {

/**
 * @brief Lock-free snapshot of the delay settings, written by the audio thread once per block
 * and read by the GUI thread (delay-tap display). Only atomics: no locks, no allocation, no GUI code.
 */
struct DelayTapState
{
    std::atomic<float> timeLeftMs  { 350.0f };
    std::atomic<float> timeRightMs { 350.0f };
    std::atomic<float> feedback    { 0.0f };   // 0..1.1
    std::atomic<float> duckGain    { 1.0f };   // current wet gain from ducking, 0..1
    std::atomic<float> wetLevel    { 0.0f };   // peak of the wet signal in the last block
    std::atomic<int>   mode        { 0 };      // 0 Clean, 1 Tape, 2 BBD, 3 Granular
    std::atomic<bool>  pingPong    { false };
    std::atomic<bool>  active      { false };  // false while the delay is bypassed
};

} // namespace wvzz::dsp
