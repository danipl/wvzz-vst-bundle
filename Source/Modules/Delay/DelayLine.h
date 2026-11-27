#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace wvzz::dsp {

/**
 * @brief Reusable fractional delay line (circular buffer, 3rd-order Lagrange read).
 *
 * No plugin-specific code and no JUCE dependency, so any module can use it
 * (WvZz Delay, the grain engine, WvZz Reverb).
 *
 * Convention inside a per-sample loop:
 *     y = line.read(d);   // returns x[n - d]
 *     line.push(x);       // stores x[n]
 * The smallest valid delay is minDelaySamples. Memory is allocated only in prepare().
 */
class DelayLine
{
public:
    static constexpr float minDelaySamples = 3.0f;

    /** Allocates the buffer for at least maxDelaySamples of delay. Not real-time safe. */
    void prepare(int maxDelaySamples)
    {
        int size = 16;
        while (size < maxDelaySamples + 8)
            size <<= 1;
        buffer.assign(static_cast<size_t>(size), 0.0f);
        mask = size - 1;
        maxDelay = static_cast<float>(maxDelaySamples);
        writeIndex = 0;
    }

    void clear() noexcept
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex = 0;
    }

    [[nodiscard]] bool isPrepared() const noexcept { return !buffer.empty(); }
    [[nodiscard]] float getMaxDelaySamples() const noexcept { return maxDelay; }

    /** Stores one sample and advances the write position. */
    void push(float sample) noexcept
    {
        buffer[static_cast<size_t>(writeIndex)] = sample;
        writeIndex = (writeIndex + 1) & mask;
    }

    /** Sample pushed `delaySamples` pushes ago, whole-sample delay. */
    [[nodiscard]] float readInteger(int delaySamples) const noexcept
    {
        return buffer[static_cast<size_t>((writeIndex - delaySamples) & mask)];
    }

    /** Sample pushed `delaySamples` pushes ago, fractional delay, 3rd-order Lagrange. */
    [[nodiscard]] float read(float delaySamples) const noexcept
    {
        const float d = std::clamp(delaySamples, minDelaySamples, maxDelay);
        const int whole = static_cast<int>(d);
        const float frac = d - static_cast<float>(whole);

        // Samples around the read point, from newest (ym1) to oldest (y2)
        const int base = writeIndex - whole;
        const float ym1 = buffer[static_cast<size_t>((base + 1) & mask)];
        const float y0  = buffer[static_cast<size_t>(base & mask)];
        const float y1  = buffer[static_cast<size_t>((base - 1) & mask)];
        const float y2  = buffer[static_cast<size_t>((base - 2) & mask)];

        const float c0 = -frac * (frac - 1.0f) * (frac - 2.0f) / 6.0f;
        const float c1 = (frac + 1.0f) * (frac - 1.0f) * (frac - 2.0f) / 2.0f;
        const float c2 = -(frac + 1.0f) * frac * (frac - 2.0f) / 2.0f;
        const float c3 = (frac + 1.0f) * frac * (frac - 1.0f) / 6.0f;

        return c0 * ym1 + c1 * y0 + c2 * y1 + c3 * y2;
    }

private:
    std::vector<float> buffer;
    int mask { 0 };
    int writeIndex { 0 };
    float maxDelay { 0.0f };
};

} // namespace wvzz::dsp
