#include "DelayDSP.h"
#include "../Grain/GrainDSP.h"

namespace wvzz::modules {

namespace {
    inline float flushDenormal(float x) noexcept { return (std::abs(x) < 1.0e-15f) ? 0.0f : x; }
}

DelayDSP::DelayDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void DelayDSP::Channel::clear() noexcept
{
    line.clear();
    for (auto& d : diffusers)
        d.clear();
    fadePosition = 1.0f;
    hpState = lpState = bbdState1 = bbdState2 = 0.0f;
    feedback = 0.0f;
}

void DelayDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Delay Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("mode"), 1 }, "Delay Mode", getDelayModeNames(), 0));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("sync"), 1 }, "Delay Sync", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("time"), 1 }, "Delay Time",
        juce::NormalisableRange<float>(minTimeMs, maxTimeMs, 0.1f, 0.35f), 350.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("sync_div"), 1 }, "Delay Sync Division", getDelaySyncDivNames(), 5)); // 1/4

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("routing"), 1 }, "Delay Routing",
        juce::StringArray{ "Stereo", "Ping-Pong" }, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("offset"), 1 }, "Delay Right Offset",
        juce::NormalisableRange<float>(0.0f, 50.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("feedback"), 1 }, "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 1.1f, 0.001f), 0.4f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("hp"), 1 }, "Delay High-Pass",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.35f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("lp"), 1 }, "Delay Low-Pass",
        juce::NormalisableRange<float>(500.0f, 20000.0f, 1.0f, 0.35f), 12000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("diffusion"), 1 }, "Delay Diffusion", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("duck"), 1 }, "Delay Ducking", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("wow"), 1 }, "Delay Wow & Flutter", 0.0f, 1.0f, 0.3f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("drive"), 1 }, "Delay Drive", 0.0f, 1.0f, 0.3f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("mix"), 1 }, "Delay Mix", 0.0f, 1.0f, 0.35f));
}

void DelayDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam    = apvts.getRawParameterValue(paramId("bypass"));
    modeParam      = apvts.getRawParameterValue(paramId("mode"));
    syncParam      = apvts.getRawParameterValue(paramId("sync"));
    timeParam      = apvts.getRawParameterValue(paramId("time"));
    syncDivParam   = apvts.getRawParameterValue(paramId("sync_div"));
    routingParam   = apvts.getRawParameterValue(paramId("routing"));
    offsetParam    = apvts.getRawParameterValue(paramId("offset"));
    feedbackParam  = apvts.getRawParameterValue(paramId("feedback"));
    hpParam        = apvts.getRawParameterValue(paramId("hp"));
    lpParam        = apvts.getRawParameterValue(paramId("lp"));
    diffusionParam = apvts.getRawParameterValue(paramId("diffusion"));
    duckParam      = apvts.getRawParameterValue(paramId("duck"));
    wowParam       = apvts.getRawParameterValue(paramId("wow"));
    driveParam     = apvts.getRawParameterValue(paramId("drive"));
    mixParam       = apvts.getRawParameterValue(paramId("mix"));
}

double DelayDSP::beatsForSyncDiv(int index) noexcept
{
    // Quarter-note beats: 1/1 = 4, dotted = x1.5, triplet = x2/3
    static constexpr double beats[] = {
        4.0, 3.0, 2.0, 4.0 / 3.0, 1.5, 1.0, 2.0 / 3.0,
        0.75, 0.5, 1.0 / 3.0, 0.375, 0.25, 1.0 / 6.0, 0.125
    };
    return beats[juce::jlimit(0, 13, index)];
}

float DelayDSP::getTargetTimeMs(double hostBpm) const noexcept
{
    const bool sync = (syncParam != nullptr) && (syncParam->load(std::memory_order_relaxed) > 0.5f);
    float timeMs = (timeParam != nullptr) ? timeParam->load(std::memory_order_relaxed) : 350.0f;

    if (sync)
    {
        const double bpm = (hostBpm > 20.0 && hostBpm < 400.0) ? hostBpm : 120.0;
        const int div = static_cast<int>((syncDivParam != nullptr) ? syncDivParam->load(std::memory_order_relaxed) : 5.0f);
        timeMs = static_cast<float>(beatsForSyncDiv(div) * 60000.0 / bpm);
    }

    return juce::jlimit(minTimeMs, maxTimeMs, timeMs);
}

double DelayDSP::getTailSeconds(double hostBpm) const noexcept
{
    const double timeSec = static_cast<double>(getTargetTimeMs(hostBpm)) * 0.001;
    const double fb = (feedbackParam != nullptr) ? static_cast<double>(feedbackParam->load(std::memory_order_relaxed)) : 0.4;

    if (fb >= 0.999)
        return 60.0;
    if (fb <= 0.001)
        return timeSec;

    // Repeats until the level is 60 dB down: fb^n = 0.001
    const double repeats = std::log(0.001) / std::log(fb);
    return juce::jlimit(timeSec, 60.0, timeSec * repeats);
}

void DelayDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // 4 seconds + 50 ms right offset + wow headroom, sized once
    const int maxSamples = static_cast<int>(sampleRate * 4.1) + 64;

    // All-pass lengths in ms (left / right differ so the two sides decorrelate)
    static constexpr float diffuserMs[2][2] = { { 5.3f, 8.9f }, { 6.1f, 9.7f } };

    for (size_t c = 0; c < channels.size(); ++c)
    {
        auto& ch = channels[c];
        ch.line.prepare(maxSamples);
        ch.diffuserLength = 0;
        for (size_t d = 0; d < ch.diffusers.size(); ++d)
        {
            ch.diffusers[d].prepare(static_cast<int>(sampleRate * diffuserMs[c][d] * 0.001));
            ch.diffuserLength += ch.diffusers[d].length();
        }
    }

    const double ramp = 0.02;
    feedbackSmoothed.reset(sampleRate, ramp);
    mixSmoothed.reset(sampleRate, ramp);
    diffusionSmoothed.reset(sampleRate, ramp);
    sendSmoothed.reset(sampleRate, 0.03);

    feedbackSmoothed.setCurrentAndTargetValue(feedbackParam != nullptr ? feedbackParam->load(std::memory_order_relaxed) : 0.4f);
    mixSmoothed.setCurrentAndTargetValue(mixParam != nullptr ? mixParam->load(std::memory_order_relaxed) : 0.35f);
    diffusionSmoothed.setCurrentAndTargetValue(diffusionParam != nullptr ? diffusionParam->load(std::memory_order_relaxed) : 0.0f);
    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    sendSmoothed.setCurrentAndTargetValue(bypassed ? 0.0f : 1.0f);

    reset();
}

void DelayDSP::reset() noexcept
{
    const float startMs = getTargetTimeMs(120.0);
    const float offsetMs = (offsetParam != nullptr) ? offsetParam->load(std::memory_order_relaxed) : 0.0f;

    for (size_t c = 0; c < channels.size(); ++c)
    {
        auto& ch = channels[c];
        ch.clear();
        const float ms = startMs + (c == 1 ? offsetMs : 0.0f);
        const float tap = static_cast<float>(sampleRate) * ms * 0.001f - static_cast<float>(ch.diffuserLength);
        ch.tapFrom = ch.tapTo = ch.glideDelay = juce::jmax(dsp::DelayLine::minDelaySamples, tap);
    }

    wowPhase = 0.0;
    flutterPhase = 0.0;
    duckEnvelope = 0.0f;
}

float DelayDSP::readTap(Channel& ch, float targetDelay, int mode, float wowOffset, float fadeStep, float glideCoeff) noexcept
{
    if (mode == Tape || mode == BBD)
    {
        // The delay glides to the new time, so the pitch of the repeats bends.
        // The slew limit keeps the read point moving forward (pitch stays between about -35% and +35%).
        ch.glideDelay += juce::jlimit(-maxGlideStep, maxGlideStep, glideCoeff * (targetDelay - ch.glideDelay));
        // Keep the Clean taps in step, so a later switch to Clean does not jump.
        ch.tapFrom = ch.tapTo = ch.glideDelay;
        ch.fadePosition = 1.0f;
        return ch.line.read(ch.glideDelay + wowOffset);
    }

    // Clean: crossfade between the old and the new delay position
    if (ch.fadePosition >= 1.0f)
    {
        if (std::abs(targetDelay - ch.tapTo) > 0.5f)
        {
            ch.tapFrom = ch.tapTo;
            ch.tapTo = targetDelay;
            ch.fadePosition = 0.0f;
        }
        else
        {
            ch.glideDelay = ch.tapTo;
            return ch.line.read(ch.tapTo);
        }
    }

    const float a = ch.line.read(ch.tapFrom);
    const float b = ch.line.read(ch.tapTo);
    const float t = ch.fadePosition;
    ch.fadePosition = juce::jmin(1.0f, ch.fadePosition + fadeStep);
    ch.glideDelay = ch.tapTo;
    return a + t * (b - a);
}

void DelayDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0 || !channels[0].line.isPrepared())
        return;

    // ---- Block-rate parameters ----
    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    int mode = static_cast<int>((modeParam != nullptr) ? modeParam->load(std::memory_order_relaxed) : 0.0f);
    if (mode == Granular && grainSource == nullptr)
        mode = Clean;
    const int routing = static_cast<int>((routingParam != nullptr) ? routingParam->load(std::memory_order_relaxed) : 0.0f);

    const float offsetMs = (offsetParam != nullptr) ? offsetParam->load(std::memory_order_relaxed) : 0.0f;
    const float fbRaw    = (feedbackParam != nullptr) ? feedbackParam->load(std::memory_order_relaxed) : 0.4f;
    const float hpRaw    = (hpParam != nullptr) ? hpParam->load(std::memory_order_relaxed) : 20.0f;
    const float lpRaw    = (lpParam != nullptr) ? lpParam->load(std::memory_order_relaxed) : 12000.0f;
    const float diffRaw  = (diffusionParam != nullptr) ? diffusionParam->load(std::memory_order_relaxed) : 0.0f;
    const float duck     = (duckParam != nullptr) ? duckParam->load(std::memory_order_relaxed) : 0.0f;
    const float wowRaw   = (wowParam != nullptr) ? wowParam->load(std::memory_order_relaxed) : 0.3f;
    const float drive    = (driveParam != nullptr) ? driveParam->load(std::memory_order_relaxed) : 0.3f;
    const float mixRaw   = (mixParam != nullptr) ? mixParam->load(std::memory_order_relaxed) : 0.35f;

    const float timeMs = juce::jlimit(minTimeMs, maxTimeMs,
        getTargetTimeMs(ctx.hostBpm) * std::pow(2.0f, ctx.getModulationOffset(ModTarget::DlyTime)));
    const float timeRightMs = juce::jlimit(minTimeMs, maxTimeMs + 50.0f, timeMs + offsetMs);

    feedbackSmoothed.setTargetValue(juce::jlimit(0.0f, 1.1f, fbRaw + ctx.getModulationOffset(ModTarget::DlyFeedback)));
    mixSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, mixRaw + ctx.getModulationOffset(ModTarget::DlyMix)));
    diffusionSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, diffRaw + ctx.getModulationOffset(ModTarget::DlyDiffusion)));
    sendSmoothed.setTargetValue(bypassed ? 0.0f : 1.0f);

    const float hpHz = juce::jlimit(20.0f, 2000.0f, hpRaw * std::pow(2.0f, ctx.getModulationOffset(ModTarget::DlyHp) * 3.0f));
    const float lpHz = juce::jlimit(500.0f, 20000.0f, lpRaw * std::pow(2.0f, ctx.getModulationOffset(ModTarget::DlyLp) * 3.0f));
    const float wow  = juce::jlimit(0.0f, 1.0f, wowRaw + ctx.getModulationOffset(ModTarget::DlyWow));

    const float fs = static_cast<float>(sampleRate);
    const float nyquistGuard = fs * 0.45f;
    const float hpCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * hpHz / fs);
    const float lpCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * juce::jmin(lpHz, nyquistGuard) / fs);

    // BBD: darker with longer times, like a slower clock
    const float bbdHz = juce::jlimit(1200.0f, 5000.0f, 3500.0f * std::sqrt(300.0f / timeMs));
    const float bbdCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * bbdHz / fs);

    const float targetTap[2] = {
        juce::jmax(dsp::DelayLine::minDelaySamples, fs * timeMs * 0.001f - static_cast<float>(channels[0].diffuserLength)),
        juce::jmax(dsp::DelayLine::minDelaySamples, fs * timeRightMs * 0.001f - static_cast<float>(channels[1].diffuserLength))
    };

    const float fadeStep = 1.0f / (fs * crossfadeMs * 0.001f);
    const float glideCoeff = 1.0f - std::exp(-1.0f / (fs * 0.15f));

    // Tape: full wow and flutter. BBD: half. Other modes: none.
    const float wowScale = (mode == Tape) ? wow : ((mode == BBD) ? wow * 0.5f : 0.0f);
    const float wowDepth = wowScale * fs * 0.0015f;       // +/- 1.5 ms
    const float flutterDepth = wowScale * fs * 0.00015f;  // +/- 0.15 ms
    const double wowInc = juce::MathConstants<double>::twoPi * 0.5 / sampleRate;
    const double flutterInc = juce::MathConstants<double>::twoPi * 6.3 / sampleRate;

    // Tape drive: more level into tanh
    const float driveGain = 1.0f + drive * 4.0f;
    const float driveMakeup = 1.0f / std::sqrt(driveGain);

    const float duckAttack  = 1.0f - std::exp(-1.0f / (fs * 0.005f));
    const float duckRelease = 1.0f - std::exp(-1.0f / (fs * 0.25f));

    if (mode == Granular)
        grainSource->updateBlock(ctx);

    // Leaving Granular mode: start the Clean taps at the current time without a fade
    if (lastMode == Granular && mode != Granular)
    {
        for (size_t c = 0; c < channels.size(); ++c)
        {
            channels[c].tapFrom = channels[c].tapTo = channels[c].glideDelay = targetTap[c];
            channels[c].fadePosition = 1.0f;
        }
    }
    lastMode = mode;

    float* left  = ctx.mainBuffer.getWritePointer(0);
    float* right = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    auto& chL = channels[0];
    auto& chR = channels[1];
    float wetPeak = 0.0f;
    float duckGain = 1.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float feedback  = feedbackSmoothed.getNextValue();
        const float mix       = mixSmoothed.getNextValue();
        const float diffusion = diffusionSmoothed.getNextValue() * 0.7f;
        const float send      = sendSmoothed.getNextValue();

        const float inL = left[i];
        const float inR = (right != nullptr) ? right[i] : inL;

        // ---- 1. Read the taps ----
        float tapL = 0.0f, tapR = 0.0f;
        if (mode == Granular)
        {
            grainSource->renderGrains(chL.line, chR.line, targetTap[0], targetTap[1], tapL, tapR);
        }
        else
        {
            const float wowOffset = wowDepth * static_cast<float>(std::sin(wowPhase))
                                  + flutterDepth * static_cast<float>(std::sin(flutterPhase));
            tapL = readTap(chL, targetTap[0], mode, wowOffset, fadeStep, glideCoeff);
            tapR = readTap(chR, targetTap[1], mode, wowOffset, fadeStep, glideCoeff);

            wowPhase += wowInc;
            if (wowPhase >= juce::MathConstants<double>::twoPi) wowPhase -= juce::MathConstants<double>::twoPi;
            flutterPhase += flutterInc;
            if (flutterPhase >= juce::MathConstants<double>::twoPi) flutterPhase -= juce::MathConstants<double>::twoPi;
        }

        // ---- 2. Diffusion, loop filters, mode colour ----
        auto colour = [&](Channel& ch, float x) noexcept {
            for (auto& d : ch.diffusers)
                x = d.process(x, diffusion);

            ch.hpState = flushDenormal(ch.hpState + hpCoeff * (x - ch.hpState));
            x -= ch.hpState;
            ch.lpState = flushDenormal(ch.lpState + lpCoeff * (x - ch.lpState));
            x = ch.lpState;

            if (mode == Tape)
            {
                // Ceiling 1/sqrt(gain), small-signal gain sqrt(gain): louder and denser with more drive
                x = std::tanh(x * driveGain) * driveMakeup;
            }
            else if (mode == BBD)
            {
                ch.bbdState1 = flushDenormal(ch.bbdState1 + bbdCoeff * (x - ch.bbdState1));
                ch.bbdState2 = flushDenormal(ch.bbdState2 + bbdCoeff * (ch.bbdState1 - ch.bbdState2));
                x = std::tanh(ch.bbdState2 * 1.5f) / 1.5f;
            }
            return x;
        };

        const float wetL = colour(chL, tapL);
        const float wetR = colour(chR, tapR);

        // ---- 3. Feedback write. The soft clipper keeps the loop bounded above 100%. ----
        const float fbL = std::tanh(wetL * feedback);
        const float fbR = std::tanh(wetR * feedback);

        if (routing == PingPong)
        {
            chL.line.push(0.5f * (inL + inR) * send + fbR);
            chR.line.push(fbL);
        }
        else
        {
            chL.line.push(inL * send + fbL);
            chR.line.push(inR * send + fbR);
        }

        // ---- 4. Ducking: follow the dry level, lower the wet output (not the loop) ----
        const float dryPeak = juce::jmax(std::abs(inL), std::abs(inR));
        duckEnvelope += ((dryPeak > duckEnvelope) ? duckAttack : duckRelease) * (dryPeak - duckEnvelope);
        duckEnvelope = flushDenormal(duckEnvelope);
        duckGain = 1.0f - duck * juce::jmin(1.0f, duckEnvelope * 4.0f);

        // ---- 5. Output. While bypassed (send -> 0) the dry signal returns to full level. ----
        const float dryGain = 1.0f - mix * send;
        const float outWetL = wetL * mix * duckGain;
        const float outWetR = wetR * mix * duckGain;
        wetPeak = juce::jmax(wetPeak, std::abs(outWetL), std::abs(outWetR));

        left[i] = inL * dryGain + outWetL;
        if (right != nullptr)
            right[i] = inR * dryGain + outWetR;
    }

    // ---- State for the delay-tap display ----
    tapState.timeLeftMs.store(timeMs, std::memory_order_relaxed);
    tapState.timeRightMs.store(timeRightMs, std::memory_order_relaxed);
    tapState.feedback.store(feedbackSmoothed.getTargetValue(), std::memory_order_relaxed);
    tapState.duckGain.store(duckGain, std::memory_order_relaxed);
    tapState.wetLevel.store(wetPeak, std::memory_order_relaxed);
    tapState.mode.store(mode, std::memory_order_relaxed);
    tapState.pingPong.store(routing == PingPong, std::memory_order_relaxed);
    tapState.active.store(!bypassed, std::memory_order_relaxed);
}

void DelayDSP::randomize(juce::Random& rng)
{
    juce::ignoreUnused(rng);
}

} // namespace wvzz::modules
