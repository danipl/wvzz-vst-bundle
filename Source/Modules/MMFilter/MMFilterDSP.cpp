#include "MMFilterDSP.h"

namespace wvzz::modules {

MMFilterDSP::MMFilterDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void MMFilterDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto freqRange    = juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.25f);
    auto timeAttRange = juce::NormalisableRange<float>(0.1f, 1000.0f, 0.1f, 0.35f);
    auto timeRelRange = juce::NormalisableRange<float>(1.0f, 5000.0f, 1.0f, 0.35f);

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "MM Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("type"), 1 }, "MM Filter Type", getMMFilterTypeNames(), 1)); // LP 24dB

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("cutoff"), 1 }, "MM Cutoff", freqRange, 1000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("res"), 1 }, "MM Resonance", 0.0f, 1.0f, 0.2f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("mix"), 1 }, "MM Mix", 0.0f, 1.0f, 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("env_att"), 1 }, "MM Env Attack", timeAttRange, 10.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("env_rel"), 1 }, "MM Env Release", timeRelRange, 150.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("env_depth"), 1 }, "MM Env Mod", -1.0f, 1.0f, 0.0f));
}

void MMFilterDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam   = apvts.getRawParameterValue(paramId("bypass"));
    typeParam     = apvts.getRawParameterValue(paramId("type"));
    cutoffParam   = apvts.getRawParameterValue(paramId("cutoff"));
    resParam      = apvts.getRawParameterValue(paramId("res"));
    mixParam      = apvts.getRawParameterValue(paramId("mix"));
    envAttParam   = apvts.getRawParameterValue(paramId("env_att"));
    envRelParam   = apvts.getRawParameterValue(paramId("env_rel"));
    envDepthParam = apvts.getRawParameterValue(paramId("env_depth"));
}

void MMFilterDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    envFollower.prepare(sampleRate);

    maxCombDelaySamples = static_cast<int>(sampleRate * 0.06) + 16;
    combBufferL.assign(static_cast<size_t>(maxCombDelaySamples), 0.0f);
    combBufferR.assign(static_cast<size_t>(maxCombDelaySamples), 0.0f);

    const double rampTimeSec = 0.015;
    cutoffSmoothed.reset(sampleRate, rampTimeSec);
    resSmoothed.reset(sampleRate, rampTimeSec);
    mixSmoothed.reset(sampleRate, rampTimeSec);
    envDepthSmoothed.reset(sampleRate, rampTimeSec);

    if (cutoffParam != nullptr)   cutoffSmoothed.setCurrentAndTargetValue(cutoffParam->load(std::memory_order_relaxed));
    if (resParam != nullptr)      resSmoothed.setCurrentAndTargetValue(resParam->load(std::memory_order_relaxed));
    if (mixParam != nullptr)      mixSmoothed.setCurrentAndTargetValue(mixParam->load(std::memory_order_relaxed));
    if (envDepthParam != nullptr) envDepthSmoothed.setCurrentAndTargetValue(envDepthParam->load(std::memory_order_relaxed));

    reset();
}

void MMFilterDSP::reset() noexcept
{
    envFollower.reset();

    svf1L.reset(); svf1R.reset();
    svf2L.reset(); svf2R.reset();

    vowelSvf1L.reset(); vowelSvf1R.reset();
    vowelSvf2L.reset(); vowelSvf2R.reset();
    vowelSvf3L.reset(); vowelSvf3R.reset();

    std::fill(combBufferL.begin(), combBufferL.end(), 0.0f);
    std::fill(combBufferR.begin(), combBufferR.end(), 0.0f);
    combWriteIndex = 0;

    ringPhase = 0.0;
    ringFeedbackSampleL = 0.0f;
    ringFeedbackSampleR = 0.0f;
}

void MMFilterDSP::setParameters(float cutoffHz, float resonanceNorm, int typeIndex,
                                float mixNorm, float envAttMs, float envRelMs,
                                float envDepthNorm, bool bypass) noexcept
{
    cutoffSmoothed.setCurrentAndTargetValue(cutoffHz);
    resSmoothed.setCurrentAndTargetValue(resonanceNorm);
    mixSmoothed.setCurrentAndTargetValue(mixNorm);
    envDepthSmoothed.setCurrentAndTargetValue(envDepthNorm);

    if (typeParam != nullptr)
        typeParam->store(static_cast<float>(typeIndex), std::memory_order_relaxed);
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);

    envFollower.setAttackRelease(envAttMs, envRelMs);
}

float MMFilterDSP::readLagrange(const std::vector<float>& buf, int currentWrite, float delaySamples) const noexcept
{
    float readPos = static_cast<float>(currentWrite) - delaySamples;
    while (readPos < 0.0f)
        readPos += static_cast<float>(maxCombDelaySamples);

    const int i1 = static_cast<int>(readPos);
    const int i0 = (i1 - 1 + maxCombDelaySamples) % maxCombDelaySamples;
    const int i2 = (i1 + 1) % maxCombDelaySamples;
    const int i3 = (i1 + 2) % maxCombDelaySamples;

    const float d = readPos - static_cast<float>(i1);

    const float y0 = buf[static_cast<size_t>(i0)];
    const float y1 = buf[static_cast<size_t>(i1)];
    const float y2 = buf[static_cast<size_t>(i2)];
    const float y3 = buf[static_cast<size_t>(i3)];

    const float c0 = -d * (d - 1.0f) * (d - 2.0f) / 6.0f;
    const float c1 = (d + 1.0f) * (d - 1.0f) * (d - 2.0f) / 2.0f;
    const float c2 = -(d + 1.0f) * d * (d - 2.0f) / 2.0f;
    const float c3 = (d + 1.0f) * d * (d - 1.0f) / 6.0f;

    return c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3;
}

void MMFilterDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels(), ctx.sidechainInputBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
        return;

    if (cutoffParam != nullptr)   cutoffSmoothed.setTargetValue(cutoffParam->load(std::memory_order_relaxed));
    if (resParam != nullptr)      resSmoothed.setTargetValue(resParam->load(std::memory_order_relaxed));
    if (mixParam != nullptr)      mixSmoothed.setTargetValue(mixParam->load(std::memory_order_relaxed));
    if (envDepthParam != nullptr) envDepthSmoothed.setTargetValue(envDepthParam->load(std::memory_order_relaxed));

    const float curCutoff   = cutoffSmoothed.skip(numSamples);
    const float curRes      = resSmoothed.skip(numSamples);
    const float curMix      = mixSmoothed.skip(numSamples);
    const float curEnvDepth = envDepthSmoothed.skip(numSamples);

    const int filterType = (typeParam != nullptr) ? static_cast<int>(typeParam->load(std::memory_order_relaxed)) : 1;
    const float envAttMs = (envAttParam != nullptr) ? envAttParam->load(std::memory_order_relaxed) : 10.0f;
    const float envRelMs = (envRelParam != nullptr) ? envRelParam->load(std::memory_order_relaxed) : 150.0f;
    envFollower.setAttackRelease(envAttMs, envRelMs);

    const float maxFreq = std::min(20000.0f, static_cast<float>(sampleRate * 0.45));
    const float modCutoffRaw = juce::jlimit(20.0f, maxFreq, curCutoff * std::pow(2.0f, ctx.getModulationOffset(ModTarget::MMCutoff) * 4.0f));
    const float modRes = juce::jlimit(0.0f, 1.0f, curRes + ctx.getModulationOffset(ModTarget::MMRes));
    const float modMix = juce::jlimit(0.0f, 1.0f, curMix + ctx.getModulationOffset(ModTarget::MMMix));

    static constexpr std::array<float, 5> formantF1 = { 800.0f,  400.0f,  300.0f,  500.0f,  350.0f };
    static constexpr std::array<float, 5> formantF2 = { 1200.0f, 1800.0f, 2200.0f, 900.0f,  750.0f };
    static constexpr std::array<float, 5> formantF3 = { 2500.0f, 2700.0f, 3000.0f, 2600.0f, 2400.0f };

    float* passL = ctx.mainBuffer.getWritePointer(0);
    float* passR = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    const float* scL = ctx.sidechainInputBuffer.getReadPointer(0);
    const float* scR = (numChannels > 1) ? ctx.sidechainInputBuffer.getReadPointer(1) : scL;

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = passL[i];
        const float inR = (passR != nullptr) ? passR[i] : inL;

        const float scMono = 0.5f * (scL[i] + scR[i]);
        const float env = envFollower.processSample(scMono);

        float sampleCutoff = modCutoffRaw;
        if (std::abs(curEnvDepth) > 0.001f)
        {
            const float octShift = curEnvDepth * env * 6.0f;
            sampleCutoff *= std::pow(2.0f, octShift);
            sampleCutoff = juce::jlimit(20.0f, maxFreq, sampleCutoff);
        }

        const float nyqRatio = sampleCutoff / maxFreq;
        const float maxQ = juce::jmap(nyqRatio * nyqRatio, 12.0f, 2.0f);
        const float q = juce::jmap(modRes, 0.7071f, maxQ);
        const float k = 1.0f / q;

        const float g = std::tan(juce::MathConstants<float>::pi * sampleCutoff / static_cast<float>(sampleRate));
        float wetL = 0.0f;
        float wetR = 0.0f;

        switch (filterType)
        {
            case 0: // LP 12dB
                wetL = svf1L.processSample(inL, g, k).lp;
                wetR = svf1R.processSample(inR, g, k).lp;
                break;
            case 1: // LP 24dB
            {
                auto s1OutL = svf1L.processSample(inL, g, k);
                auto s1OutR = svf1R.processSample(inR, g, k);
                const float st1L = std::tanh(s1OutL.lp);
                const float st1R = std::tanh(s1OutR.lp);
                wetL = svf2L.processSample(st1L, g, k).lp;
                wetR = svf2R.processSample(st1R, g, k).lp;
                break;
            }
            case 2: // BP 12dB
                wetL = svf1L.processSample(inL, g, k).bp;
                wetR = svf1R.processSample(inR, g, k).bp;
                break;
            case 3: // BP 24dB
            {
                auto s1OutL = svf1L.processSample(inL, g, k);
                auto s1OutR = svf1R.processSample(inR, g, k);
                wetL = svf2L.processSample(s1OutL.bp, g, k).bp;
                wetR = svf2R.processSample(s1OutR.bp, g, k).bp;
                break;
            }
            case 4: // HP 12dB
                wetL = svf1L.processSample(inL, g, k).hp;
                wetR = svf1R.processSample(inR, g, k).hp;
                break;
            case 5: // HP 24dB
            {
                auto s1OutL = svf1L.processSample(inL, g, k);
                auto s1OutR = svf1R.processSample(inR, g, k);
                wetL = svf2L.processSample(s1OutL.hp, g, k).hp;
                wetR = svf2R.processSample(s1OutR.hp, g, k).hp;
                break;
            }
            case 6: // Notch
            {
                auto outL = svf1L.processSample(inL, g, k);
                auto outR = svf1R.processSample(inR, g, k);
                wetL = outL.lp + outL.hp;
                wetR = outR.lp + outR.hp;
                break;
            }
            case 7: // Peak
            {
                auto outL = svf1L.processSample(inL, g, k);
                auto outR = svf1R.processSample(inR, g, k);
                wetL = inL + outL.bp * (q * 1.2f);
                wetR = inR + outR.bp * (q * 1.2f);
                break;
            }
            case 8: // Comb +
            case 9: // Comb -
            {
                const float delayTimeSec = 1.0f / sampleCutoff;
                const float delaySamples = juce::jlimit(1.0f, static_cast<float>(maxCombDelaySamples - 4), delayTimeSec * static_cast<float>(sampleRate));
                const float sign = (filterType == 8) ? 1.0f : -1.0f;
                const float feedback = sign * (0.15f + 0.82f * modRes);

                const float delayedL = readLagrange(combBufferL, combWriteIndex, delaySamples);
                const float delayedR = readLagrange(combBufferR, combWriteIndex, delaySamples);

                const float writeL = inL + std::tanh(feedback * delayedL);
                const float writeR = inR + std::tanh(feedback * delayedR);

                combBufferL[static_cast<size_t>(combWriteIndex)] = writeL;
                combBufferR[static_cast<size_t>(combWriteIndex)] = writeR;

                wetL = 0.5f * (inL + delayedL);
                wetR = 0.5f * (inR + delayedR);

                if (++combWriteIndex >= maxCombDelaySamples)
                    combWriteIndex = 0;
                break;
            }
            case 10: // RingMod
            {
                const double phaseInc = (juce::MathConstants<double>::twoPi * sampleCutoff) / sampleRate;
                const float carrier = static_cast<float>(std::sin(ringPhase));
                ringPhase += phaseInc;
                if (ringPhase >= juce::MathConstants<double>::twoPi)
                    ringPhase -= juce::MathConstants<double>::twoPi;

                const float fbL = std::tanh(modRes * 0.8f * ringFeedbackSampleL);
                const float fbR = std::tanh(modRes * 0.8f * ringFeedbackSampleR);

                wetL = inL * (carrier + fbL);
                wetR = inR * (carrier + fbR);

                ringFeedbackSampleL = wetL;
                ringFeedbackSampleR = wetR;
                break;
            }
            case 11: // Vowel / Formant
            {
                const float normCut = juce::jlimit(0.0f, 4.0f, std::log2(sampleCutoff / 20.0f) * (4.0f / 9.965f));
                const int idx = juce::jlimit(0, 3, static_cast<int>(normCut));
                const float frac = normCut - static_cast<float>(idx);

                const float f1 = (1.0f - frac) * formantF1[static_cast<size_t>(idx)] + frac * formantF1[static_cast<size_t>(idx + 1)];
                const float f2 = (1.0f - frac) * formantF2[static_cast<size_t>(idx)] + frac * formantF2[static_cast<size_t>(idx + 1)];
                const float f3 = (1.0f - frac) * formantF3[static_cast<size_t>(idx)] + frac * formantF3[static_cast<size_t>(idx + 1)];

                const float qVowel = 4.0f + modRes * 14.0f;
                const float kVowel = 1.0f / qVowel;

                const float g1 = std::tan(juce::MathConstants<float>::pi * f1 / static_cast<float>(sampleRate));
                const float g2 = std::tan(juce::MathConstants<float>::pi * f2 / static_cast<float>(sampleRate));
                const float g3 = std::tan(juce::MathConstants<float>::pi * f3 / static_cast<float>(sampleRate));

                const float vL1 = vowelSvf1L.processSample(inL, g1, kVowel).bp;
                const float vL2 = vowelSvf2L.processSample(inL, g2, kVowel).bp;
                const float vL3 = vowelSvf3L.processSample(inL, g3, kVowel).bp;

                const float vR1 = vowelSvf1R.processSample(inR, g1, kVowel).bp;
                const float vR2 = vowelSvf2R.processSample(inR, g2, kVowel).bp;
                const float vR3 = vowelSvf3R.processSample(inR, g3, kVowel).bp;

                wetL = (vL1 + vL2 * 0.7f + vL3 * 0.4f) * 1.5f;
                wetR = (vR1 + vR2 * 0.7f + vR3 * 0.4f) * 1.5f;
                break;
            }
        }

        passL[i] = (1.0f - modMix) * inL + modMix * wetL;
        if (passR != nullptr)
            passR[i] = (1.0f - modMix) * inR + modMix * wetR;
    }
}

void MMFilterDSP::randomize(juce::Random& rng)
{
    if (cutoffSmoothed.isSmoothing()) cutoffSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 4000.0f + 200.0f);
    if (resSmoothed.isSmoothing()) resSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.7f);
    if (mixSmoothed.isSmoothing()) mixSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 0.5f + 0.5f);
}

} // namespace wvzz::modules
