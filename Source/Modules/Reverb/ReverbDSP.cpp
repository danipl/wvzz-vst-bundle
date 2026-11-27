#include "ReverbDSP.h"

namespace wvzz::modules {

namespace {
    inline float flushDenormal(float x) noexcept { return (std::abs(x) < 1.0e-15f) ? 0.0f : x; }

    // Line lengths in ms, chosen without common factors so the echoes do not line up.
    constexpr float plateMs[ReverbDSP::numLines] = { 11.3f, 14.9f, 18.1f, 21.7f, 25.3f, 29.9f, 34.1f, 38.3f };
    constexpr float hallMs[ReverbDSP::numLines]  = { 37.3f, 43.7f, 51.1f, 59.3f, 67.9f, 76.1f, 85.7f, 97.3f };

    // Input diffuser lengths in ms (left / right)
    constexpr float inputDiffuserMs[2][4] = { { 4.7f, 3.6f, 12.7f, 9.3f }, { 5.1f, 3.3f, 11.9f, 9.7f } };

    // Tail modulation: rate in Hz for each line; 0 = this line is not modulated
    constexpr float modRateHz[ReverbDSP::numLines] = { 0.31f, 0.0f, 0.47f, 0.0f, 0.71f, 0.0f, 0.97f, 0.0f };

    // Output tap signs: mixed signs make the left and right sums differ
    constexpr float outSign[ReverbDSP::numLines] = { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f };
}

ReverbDSP::ReverbDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

float ReverbDSP::baseLineMs(int mode, int line) noexcept
{
    const int i = juce::jlimit(0, numLines - 1, line);
    return (mode == Plate) ? plateMs[i] : hallMs[i];
}

float ReverbDSP::semitonesForInterval(int index) noexcept
{
    static constexpr float semis[] = { 12.0f, 7.0f, 19.0f, 24.0f, -12.0f };
    return semis[juce::jlimit(0, 4, index)];
}

void ReverbDSP::mixMatrix(std::array<float, numLines>& v) noexcept
{
    // Fast Walsh-Hadamard transform: 3 butterfly stages for 8 values
    for (int step = 1; step < numLines; step <<= 1)
    {
        for (int i = 0; i < numLines; i += step << 1)
        {
            for (int j = i; j < i + step; ++j)
            {
                const float a = v[static_cast<size_t>(j)];
                const float b = v[static_cast<size_t>(j + step)];
                v[static_cast<size_t>(j)] = a + b;
                v[static_cast<size_t>(j + step)] = a - b;
            }
        }
    }
    constexpr float scale = 0.35355339059f; // 1 / sqrt(8)
    for (auto& x : v)
        x *= scale;
}

void ReverbDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Reverb Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("mode"), 1 }, "Reverb Mode", getReverbModeNames(), 1)); // Hall

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("predelay"), 1 }, "Reverb Pre-Delay",
        juce::NormalisableRange<float>(0.0f, 250.0f, 0.1f, 0.5f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("size"), 1 }, "Reverb Size", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("decay"), 1 }, "Reverb Decay",
        juce::NormalisableRange<float>(minDecaySeconds, maxDecaySeconds, 0.01f, 0.4f), 2.5f,
        juce::AudioParameterFloatAttributes().withLabel("s")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("lodamp"), 1 }, "Reverb Low Damping",
        juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f, 0.4f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("hidamp"), 1 }, "Reverb High Damping",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.4f), 6000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("diffusion"), 1 }, "Reverb Diffusion", 0.0f, 1.0f, 0.7f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("mod"), 1 }, "Reverb Tail Modulation", 0.0f, 1.0f, 0.25f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("width"), 1 }, "Reverb Width", 0.0f, 1.0f, 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("shimmer"), 1 }, "Reverb Shimmer Amount", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("interval"), 1 }, "Reverb Shimmer Interval", getShimmerIntervalNames(), 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("mix"), 1 }, "Reverb Mix", 0.0f, 1.0f, 0.3f));
}

void ReverbDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam    = apvts.getRawParameterValue(paramId("bypass"));
    modeParam      = apvts.getRawParameterValue(paramId("mode"));
    predelayParam  = apvts.getRawParameterValue(paramId("predelay"));
    sizeParam      = apvts.getRawParameterValue(paramId("size"));
    decayParam     = apvts.getRawParameterValue(paramId("decay"));
    lodampParam    = apvts.getRawParameterValue(paramId("lodamp"));
    hidampParam    = apvts.getRawParameterValue(paramId("hidamp"));
    diffusionParam = apvts.getRawParameterValue(paramId("diffusion"));
    modParam       = apvts.getRawParameterValue(paramId("mod"));
    widthParam     = apvts.getRawParameterValue(paramId("width"));
    shimmerParam   = apvts.getRawParameterValue(paramId("shimmer"));
    intervalParam  = apvts.getRawParameterValue(paramId("interval"));
    mixParam       = apvts.getRawParameterValue(paramId("mix"));
}

double ReverbDSP::getTailSeconds() const noexcept
{
    const double predelay = (predelayParam != nullptr) ? static_cast<double>(predelayParam->load(std::memory_order_relaxed)) * 0.001 : 0.02;
    const double decay = (decayParam != nullptr) ? static_cast<double>(decayParam->load(std::memory_order_relaxed)) : 2.5;
    return predelay + decay;
}

void ReverbDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    for (size_t c = 0; c < 2; ++c)
    {
        predelayLines[c].prepare(static_cast<int>(sampleRate * 0.3));
        shimmerLines[c].prepare(static_cast<int>(sampleRate * 0.5));
        for (size_t d = 0; d < 4; ++d)
            inputDiffusers[c][d].prepare(static_cast<int>(sampleRate * inputDiffuserMs[c][d] * 0.001));
    }

    // Longest line: 97.3 ms x 1.5 (size) + modulation headroom
    for (auto& line : lines)
        line.prepare(static_cast<int>(sampleRate * 0.16) + 64);

    shimmerEngine.prepare(sampleRate);

    mixSmoothed.reset(sampleRate, 0.02);
    widthSmoothed.reset(sampleRate, 0.02);
    sendSmoothed.reset(sampleRate, 0.03);
    shimmerSmoothed.reset(sampleRate, 0.05);
    modeFadeSmoothed.reset(sampleRate, 0.03);

    mixSmoothed.setCurrentAndTargetValue(mixParam != nullptr ? mixParam->load(std::memory_order_relaxed) : 0.3f);
    widthSmoothed.setCurrentAndTargetValue(widthParam != nullptr ? widthParam->load(std::memory_order_relaxed) : 1.0f);
    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    sendSmoothed.setCurrentAndTargetValue(bypassed ? 0.0f : 1.0f);
    modeFadeSmoothed.setCurrentAndTargetValue(1.0f);

    activeMode = pendingMode = juce::jlimit(0, 2, static_cast<int>((modeParam != nullptr) ? modeParam->load(std::memory_order_relaxed) : 1.0f));
    shimmerSmoothed.setCurrentAndTargetValue(0.0f);

    reset();
}

void ReverbDSP::clearNetwork() noexcept
{
    for (auto& line : lines)
        if (line.isPrepared())
            line.clear();
    lpState.fill(0.0f);
    hpState.fill(0.0f);
    for (auto& line : shimmerLines)
        if (line.isPrepared())
            line.clear();
    shimmerEngine.reset();
    shimmerLp = { 0.0f, 0.0f };
    for (auto& channel : inputDiffusers)
        for (auto& d : channel)
            d.clear();
}

void ReverbDSP::reset() noexcept
{
    clearNetwork();
    for (auto& line : predelayLines)
        if (line.isPrepared())
            line.clear();

    const float size = (sizeParam != nullptr) ? sizeParam->load(std::memory_order_relaxed) : 0.5f;
    const float scale = 0.5f + juce::jlimit(0.0f, 1.0f, size);
    for (int i = 0; i < numLines; ++i)
        lineLength[static_cast<size_t>(i)] = static_cast<float>(sampleRate) * baseLineMs(activeMode, i) * 0.001f * scale;

    const float predelaySamples = static_cast<float>(sampleRate) * 0.001f
        * ((predelayParam != nullptr) ? predelayParam->load(std::memory_order_relaxed) : 20.0f);
    predelayGlide = { predelaySamples, predelaySamples };
    modPhase.fill(0.0);
}

void ReverbDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0 || !lines[0].isPrepared())
        return;

    // ---- Block-rate parameters ----
    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    const int requestedMode = juce::jlimit(0, 2, static_cast<int>((modeParam != nullptr) ? modeParam->load(std::memory_order_relaxed) : 1.0f));

    const float predelayRaw = (predelayParam != nullptr) ? predelayParam->load(std::memory_order_relaxed) : 20.0f;
    const float sizeRaw     = (sizeParam != nullptr) ? sizeParam->load(std::memory_order_relaxed) : 0.5f;
    const float decayRaw    = (decayParam != nullptr) ? decayParam->load(std::memory_order_relaxed) : 2.5f;
    const float lodampRaw   = (lodampParam != nullptr) ? lodampParam->load(std::memory_order_relaxed) : 100.0f;
    const float hidampRaw   = (hidampParam != nullptr) ? hidampParam->load(std::memory_order_relaxed) : 6000.0f;
    const float diffRaw     = (diffusionParam != nullptr) ? diffusionParam->load(std::memory_order_relaxed) : 0.7f;
    const float modRaw      = (modParam != nullptr) ? modParam->load(std::memory_order_relaxed) : 0.25f;
    const float widthRaw    = (widthParam != nullptr) ? widthParam->load(std::memory_order_relaxed) : 1.0f;
    const float shimmerRaw  = (shimmerParam != nullptr) ? shimmerParam->load(std::memory_order_relaxed) : 0.5f;
    const int interval      = static_cast<int>((intervalParam != nullptr) ? intervalParam->load(std::memory_order_relaxed) : 0.0f);
    const float mixRaw      = (mixParam != nullptr) ? mixParam->load(std::memory_order_relaxed) : 0.3f;

    const float fs = static_cast<float>(sampleRate);

    const float predelayMs = juce::jlimit(0.0f, 250.0f, predelayRaw + ctx.getModulationOffset(ModTarget::RevPredelay) * 100.0f);
    const float size       = juce::jlimit(0.0f, 1.0f, sizeRaw + ctx.getModulationOffset(ModTarget::RevSize));
    const float decay      = juce::jlimit(minDecaySeconds, maxDecaySeconds, decayRaw * std::pow(2.0f, ctx.getModulationOffset(ModTarget::RevDecay) * 2.0f));
    const float hiHz       = juce::jlimit(1000.0f, 20000.0f, hidampRaw * std::pow(2.0f, ctx.getModulationOffset(ModTarget::RevHiDamp) * 3.0f));
    const float loHz       = juce::jlimit(20.0f, 1000.0f, lodampRaw * std::pow(2.0f, ctx.getModulationOffset(ModTarget::RevLoDamp) * 3.0f));
    const float diffusion  = juce::jlimit(0.0f, 1.0f, diffRaw + ctx.getModulationOffset(ModTarget::RevDiffusion));
    const float modDepth   = juce::jlimit(0.0f, 1.0f, modRaw + ctx.getModulationOffset(ModTarget::RevMod));

    mixSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, mixRaw + ctx.getModulationOffset(ModTarget::RevMix)));
    widthSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, widthRaw + ctx.getModulationOffset(ModTarget::RevWidth)));
    sendSmoothed.setTargetValue(bypassed ? 0.0f : 1.0f);

    // ---- Mode change: fade the wet signal out, clear the network, switch, fade in ----
    if (requestedMode != pendingMode)
    {
        pendingMode = requestedMode;
        modeFadeSmoothed.setTargetValue(0.0f);
    }
    if (pendingMode != activeMode && !modeFadeSmoothed.isSmoothing() && modeFadeSmoothed.getCurrentValue() <= 0.0f)
    {
        activeMode = pendingMode;
        clearNetwork();
        const float scaleNow = 0.5f + size;
        for (int i = 0; i < numLines; ++i)
            lineLength[static_cast<size_t>(i)] = fs * baseLineMs(activeMode, i) * 0.001f * scaleNow;
        modeFadeSmoothed.setTargetValue(1.0f);
    }
    const int mode = activeMode;

    shimmerSmoothed.setTargetValue((mode == Shimmer) ? juce::jlimit(0.0f, 1.0f, shimmerRaw + ctx.getModulationOffset(ModTarget::RevShimmer)) : 0.0f);

    // ---- Line lengths, decay gains, damping ----
    const float sizeScale = 0.5f + size; // 0.5 .. 1.5
    std::array<float, numLines> targetLength {};
    std::array<float, numLines> lineGain {};
    std::array<float, numLines> dampAmount {};   // share of the damped range removed on one pass of this line
    const float dampDbPerSecond = fullDampingDbPerSecond(decay);
    float meanLength = 0.0f;
    float meanGainSq = 0.0f;
    for (int i = 0; i < numLines; ++i)
    {
        const auto idx = static_cast<size_t>(i);
        targetLength[idx] = fs * baseLineMs(mode, i) * 0.001f * sizeScale;
        // -60 dB after `decay` seconds: gain per pass = 10^(-3 * length / (decay * fs))
        lineGain[idx] = std::pow(10.0f, -3.0f * targetLength[idx] / (decay * fs));
        // Damping is set per second, not per pass: a short line removes less on each pass than a long one.
        dampAmount[idx] = 1.0f - std::pow(10.0f, -(dampDbPerSecond * targetLength[idx] / fs) / 20.0f);
        meanLength += targetLength[idx];
        meanGainSq += lineGain[idx] * lineGain[idx];
    }
    meanLength /= static_cast<float>(numLines);
    meanGainSq /= static_cast<float>(numLines);

    // Feed scaling: keeps the wet level steady across the Decay range
    const float feedGain = std::sqrt(juce::jmax(1.0e-4f, 1.0f - meanGainSq));

    // One-pole trackers split off the range above High Damp and below Low Damp.
    // At the end positions (20 kHz / 20 Hz) the damping is off, so the Decay knob is exact.
    const bool hiDampOn = hiHz < 19990.0f;
    const bool loDampOn = loHz > 20.5f;
    const float lpCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * juce::jmin(hiHz, fs * 0.45f) / fs);
    const float hpCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * loHz / fs);
    const float lengthGlide = 1.0f - std::exp(-1.0f / (fs * 0.1f));
    const float predelayTarget = juce::jmax(dsp::DelayLine::minDelaySamples, fs * predelayMs * 0.001f);
    const float predelayCoeff = 1.0f - std::exp(-1.0f / (fs * 0.05f));

    // Plate: strong input diffusion. Hall and Shimmer: a little less, so the build-up is slower.
    const float diffusionGain = ((mode == Plate) ? 0.75f : 0.62f) * diffusion;
    const float modSamples = modDepth * 12.0f * (fs / 48000.0f);

    dsp::GrainEngine::Settings grainSettings;
    grainSettings.sizeSamples = fs * 0.09f;
    grainSettings.grainsPerSecond = 32.0f;
    grainSettings.pitchRatio = std::pow(2.0f, semitonesForInterval(interval) / 12.0f);
    grainSettings.spraySamples = fs * 0.02f;
    grainSettings.reverseChance = 0.0f;
    grainSettings.stereoSpread = 0.6f;
    const float shimmerBase = fs * 0.12f;
    const float shimmerLpCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * 9000.0f / fs);

    float* left  = ctx.mainBuffer.getWritePointer(0);
    float* right = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    const bool hasEarly = ctx.hasAuxSplit && ctx.auxBandRejectBuffer.getNumChannels() >= 2
                          && ctx.auxBandRejectBuffer.getNumSamples() >= numSamples;
    const float* earlyL = hasEarly ? ctx.auxBandRejectBuffer.getReadPointer(0) : nullptr;
    const float* earlyR = hasEarly ? ctx.auxBandRejectBuffer.getReadPointer(1) : nullptr;

    float wetPeak = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float mix      = mixSmoothed.getNextValue();
        const float width    = widthSmoothed.getNextValue();
        const float send     = sendSmoothed.getNextValue();
        const float shimmer  = shimmerSmoothed.getNextValue();
        const float modeFade = modeFadeSmoothed.getNextValue();

        const float inL = left[i];
        const float inR = (right != nullptr) ? right[i] : inL;
        const float erL = hasEarly ? earlyL[i] : 0.0f;
        const float erR = hasEarly ? earlyR[i] : 0.0f;

        // ---- 1. Pre-delay on dry + early reflections ----
        float feed[2];
        for (size_t c = 0; c < 2; ++c)
        {
            predelayGlide[c] += predelayCoeff * (predelayTarget - predelayGlide[c]);
            feed[c] = predelayLines[c].read(predelayGlide[c]);
        }
        predelayLines[0].push((inL + erL) * send);
        predelayLines[1].push((inR + erR) * send);

        // A pre-delay of 0 ms still passes the minimum line delay (3 samples)

        // ---- 2. Shimmer return: pitch-shifted tail, low-passed and soft-clipped ----
        if (shimmer > 0.0001f)
        {
            float grainL = 0.0f, grainR = 0.0f;
            shimmerEngine.process(shimmerLines[0], shimmerLines[1], shimmerBase, shimmerBase, grainSettings, grainL, grainR);
            shimmerLp[0] = flushDenormal(shimmerLp[0] + shimmerLpCoeff * (grainL - shimmerLp[0]));
            shimmerLp[1] = flushDenormal(shimmerLp[1] + shimmerLpCoeff * (grainR - shimmerLp[1]));
            // tanh bounds what the pitch path can put back, so the loop cannot run away
            feed[0] += 0.5f * shimmer * std::tanh(shimmerLp[0] * 2.0f);
            feed[1] += 0.5f * shimmer * std::tanh(shimmerLp[1] * 2.0f);
        }

        // ---- 3. Input diffusion ----
        for (size_t c = 0; c < 2; ++c)
            for (auto& d : inputDiffusers[c])
                feed[c] = d.process(feed[c], diffusionGain);

        // ---- 4. Feedback Delay Network ----
        std::array<float, numLines> v {};
        float outL = 0.0f, outR = 0.0f;
        for (int n = 0; n < numLines; ++n)
        {
            const auto idx = static_cast<size_t>(n);
            lineLength[idx] += lengthGlide * (targetLength[idx] - lineLength[idx]);

            float readDelay = lineLength[idx];
            if (modRateHz[n] > 0.0f && modSamples > 0.0f)
            {
                readDelay += modSamples * static_cast<float>(std::sin(modPhase[idx]));
                modPhase[idx] += juce::MathConstants<double>::twoPi * static_cast<double>(modRateHz[n]) / sampleRate;
                if (modPhase[idx] >= juce::MathConstants<double>::twoPi)
                    modPhase[idx] -= juce::MathConstants<double>::twoPi;
            }

            float x = lines[idx].read(readDelay);

            // Damping: remove a share of the highs above hiHz and of the lows below loHz on every pass
            if (hiDampOn)
            {
                lpState[idx] = flushDenormal(lpState[idx] + lpCoeff * (x - lpState[idx]));
                x -= dampAmount[idx] * (x - lpState[idx]);   // x - lowpass(x) = the highs
            }
            if (loDampOn)
            {
                hpState[idx] = flushDenormal(hpState[idx] + hpCoeff * (x - hpState[idx]));
                x -= dampAmount[idx] * hpState[idx];         // lowpass(x) at loHz = the lows
            }

            if ((n & 1) == 0) outL += outSign[n] * x;
            else              outR += outSign[n] * x;

            v[idx] = x * lineGain[idx];
        }

        mixMatrix(v);

        for (int n = 0; n < numLines; ++n)
        {
            const auto idx = static_cast<size_t>(n);
            const float input = (((n & 1) == 0) ? feed[0] : feed[1]) * feedGain;
            lines[idx].push(input + v[idx]);
        }

        // 4 lines per side
        outL *= 0.5f;
        outR *= 0.5f;

        // ---- 5. Shimmer send: record the network output for the grain engine ----
        shimmerLines[0].push(outL);
        shimmerLines[1].push(outR);

        // -4.4 dB of headroom on the wet output for long decays with loud input.
        // Applied after the shimmer send, so the shimmer amount does not depend on it.
        outL *= 0.6f;
        outR *= 0.6f;

        // ---- 6. Width (mid/side), early reflections, mix ----
        const float mid = 0.5f * (outL + outR);
        const float side = 0.5f * (outL - outR) * width;
        // The early reflections follow `send`, so a bypassed reverb does not keep playing them.
        const float wetL = ((mid + side) * modeFade + erL * send) * mix;
        const float wetR = ((mid - side) * modeFade + erR * send) * mix;
        wetPeak = juce::jmax(wetPeak, std::abs(wetL), std::abs(wetR));

        // While bypassed (send -> 0) the dry signal returns to full level and the tail fades on its own.
        const float dryGain = 1.0f - mix * send;
        left[i] = inL * dryGain + wetL;
        if (right != nullptr)
            right[i] = inR * dryGain + wetR;
    }

    // ---- State for the decay-curve display ----
    decayState.decaySeconds.store(decay, std::memory_order_relaxed);
    decayState.predelayMs.store(predelayMs, std::memory_order_relaxed);
    decayState.hiDampHz.store(hiHz, std::memory_order_relaxed);
    decayState.loDampHz.store(loHz, std::memory_order_relaxed);
    decayState.meanLineMs.store(meanLength * 1000.0f / fs, std::memory_order_relaxed);
    decayState.shimmer.store(shimmerSmoothed.getTargetValue(), std::memory_order_relaxed);
    decayState.wetLevel.store(wetPeak, std::memory_order_relaxed);
    decayState.mode.store(mode, std::memory_order_relaxed);
    decayState.active.store(!bypassed, std::memory_order_relaxed);
}

void ReverbDSP::randomize(juce::Random& rng)
{
    juce::ignoreUnused(rng);
}

} // namespace wvzz::modules
