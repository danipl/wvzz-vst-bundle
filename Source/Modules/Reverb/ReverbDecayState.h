#pragma once

#include <atomic>

namespace wvzz::dsp {

/**
 * @brief Lock-free snapshot of the reverb settings, written by the audio thread once per block
 * and read by the GUI thread (decay-curve display). Only atomics: no locks, no allocation, no GUI code.
 */
struct ReverbDecayState
{
    std::atomic<float> decaySeconds   { 2.5f };
    std::atomic<float> predelayMs     { 20.0f };
    std::atomic<float> hiDampHz       { 6000.0f };
    std::atomic<float> loDampHz       { 100.0f };
    std::atomic<float> meanLineMs     { 60.0f };   // mean FDN line length: sets how often the damping acts
    std::atomic<float> earlyMs        { 0.0f };    // early-reflection pattern length, 0 when off
    std::atomic<float> shimmer        { 0.0f };
    std::atomic<float> wetLevel       { 0.0f };    // peak of the wet signal in the last block
    std::atomic<int>   mode           { 0 };       // 0 Plate, 1 Hall, 2 Shimmer
    std::atomic<bool>  active         { false };   // false while the reverb is bypassed
};

} // namespace wvzz::dsp
