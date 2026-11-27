#include "GrainEngine.h"
#include <cmath>

namespace wvzz::dsp {

void GrainEngine::prepare(double newSampleRate) noexcept
{
    sampleRate = newSampleRate;
    reset();
}

void GrainEngine::reset() noexcept
{
    for (auto& v : voices)
        v.active = false;
    samplesUntilNextGrain = 0.0f;
    activeVoices = 0;
    rngState = 0x9E3779B9u;
}

float GrainEngine::nextRandom() noexcept
{
    // xorshift32: fast, deterministic, no allocation
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return static_cast<float>(rngState & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

void GrainEngine::spawn(const DelayLine& lineL, float baseDelayL, float baseDelayR, const Settings& settings) noexcept
{
    Voice* freeVoice = nullptr;
    for (auto& v : voices)
    {
        if (!v.active)
        {
            freeVoice = &v;
            break;
        }
    }
    if (freeVoice == nullptr) // all 32 voices busy: skip this grain
        return;

    auto& v = *freeVoice;
    const float maxDelay = lineL.getMaxDelaySamples() - 4.0f;
    const float minDelay = DelayLine::minDelaySamples + 1.0f;

    v.length = std::max(32.0f, settings.sizeSamples);
    v.age = 0.0f;

    const bool reversed = nextRandom() < settings.reverseChance;
    const float ratio = std::clamp(settings.pitchRatio, 0.25f, 4.0f);
    // Forward grain: the read point moves at `ratio` samples per sample, so the delay changes by 1 - ratio.
    // Reversed grain: the read point moves backwards, so the delay grows by 1 + ratio.
    v.delayStep = reversed ? (1.0f + ratio) : (1.0f - ratio);

    const float spray = nextRandom() * std::max(0.0f, settings.spraySamples);
    const float travel = v.delayStep * v.length; // total change of the delay over the grain

    auto startDelay = [&](float base) {
        float start = base + spray;
        // Keep the whole grain inside the valid delay range.
        if (travel < 0.0f)
            start = std::max(start, minDelay - travel); // delay shrinks: start far enough back
        else
            start = std::min(start, maxDelay - travel); // delay grows: leave room at the far end
        return std::clamp(start, minDelay, maxDelay);
    };
    v.delayL = startDelay(baseDelayL);
    v.delayR = startDelay(baseDelayR);

    // Random pan with constant power; spread 0 = centre
    const float pan = 0.5f + (nextRandom() - 0.5f) * std::clamp(settings.stereoSpread, 0.0f, 1.0f);
    v.gainL = std::sqrt(2.0f * (1.0f - pan));
    v.gainR = std::sqrt(2.0f * pan);

    v.active = true;
}

void GrainEngine::process(const DelayLine& lineL, const DelayLine& lineR,
                          float baseDelayL, float baseDelayR,
                          const Settings& settings, float& outL, float& outR) noexcept
{
    const float density = std::clamp(settings.grainsPerSecond, 0.5f, 200.0f);

    samplesUntilNextGrain -= 1.0f;
    if (samplesUntilNextGrain <= 0.0f)
    {
        spawn(lineL, baseDelayL, baseDelayR, settings);
        // +/-25% timing jitter avoids a fixed-rate buzz
        const float interval = static_cast<float>(sampleRate) / density;
        samplesUntilNextGrain += interval * (0.75f + 0.5f * nextRandom());
    }

    const float maxDelay = lineL.getMaxDelaySamples() - 2.0f;
    float sumL = 0.0f;
    float sumR = 0.0f;
    int active = 0;

    for (auto& v : voices)
    {
        if (!v.active)
            continue;

        ++active;
        const float window = 0.5f - 0.5f * std::cos(6.283185307f * (v.age / v.length)); // Hann
        sumL += lineL.read(std::clamp(v.delayL, DelayLine::minDelaySamples, maxDelay)) * window * v.gainL;
        sumR += lineR.read(std::clamp(v.delayR, DelayLine::minDelaySamples, maxDelay)) * window * v.gainR;

        v.delayL += v.delayStep;
        v.delayR += v.delayStep;
        v.age += 1.0f;
        if (v.age >= v.length)
            v.active = false;
    }

    activeVoices = active;

    // Expected overlap = grains per second x grain length in seconds. Scale so the level stays steady.
    const float overlap = density * settings.sizeSamples / static_cast<float>(sampleRate);
    // Overlapping Hann grains of similar audio add up to about overlap / 2. The 0.75 exponent sits
    // between fully correlated (1.0) and uncorrelated (0.5) grains.
    const float gain = 1.0f / std::pow(std::max(1.0f, std::min(overlap, static_cast<float>(maxVoices)) * 0.5f), 0.75f);
    outL = sumL * gain;
    outR = sumR * gain;
}

} // namespace wvzz::dsp
