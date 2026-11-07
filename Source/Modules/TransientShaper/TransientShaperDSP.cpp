#include "TransientShaperDSP.h"

namespace wvzz::modules {

TransientShaperDSP::TransientShaperDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void TransientShaperDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Transient Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("attack"), 1 }, "Transient Attack", -1.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("sustain"), 1 }, "Transient Sustain", -1.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("speed"), 1 }, "Transient Speed",
        juce::StringArray{ "Fast", "Punch" }, 0));

    juce::NormalisableRange<float> gateRange(-80.0f, 0.0f, 0.1f);
    gateRange.setSkewForCentre(-24.0f);

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("gate"), 1 }, "Gate Floor", gateRange, -80.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
}

void TransientShaperDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam  = apvts.getRawParameterValue(paramId("bypass"));
    attackParam  = apvts.getRawParameterValue(paramId("attack"));
    sustainParam = apvts.getRawParameterValue(paramId("sustain"));
    speedParam   = apvts.getRawParameterValue(paramId("speed"));
    gateParam    = apvts.getRawParameterValue(paramId("gate"));
}

void TransientShaperDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const double rampTimeSec = 0.015;
    attackSmoothed.reset(sampleRate, rampTimeSec);
    sustainSmoothed.reset(sampleRate, rampTimeSec);
    gateSmoothed.reset(sampleRate, rampTimeSec);

    if (attackParam != nullptr)  attackSmoothed.setCurrentAndTargetValue(attackParam->load(std::memory_order_relaxed));
    if (sustainParam != nullptr) sustainSmoothed.setCurrentAndTargetValue(sustainParam->load(std::memory_order_relaxed));
    if (gateParam != nullptr)    gateSmoothed.setCurrentAndTargetValue(gateParam->load(std::memory_order_relaxed));
    else                         gateSmoothed.setCurrentAndTargetValue(-80.0f);

    reset();
}

void TransientShaperDSP::reset() noexcept
{
    envFastL = 0.0f; envFastR = 0.0f;
    envSlowL = 0.0f; envSlowR = 0.0f;
    gateEnv = 0.0f;
    gateGain = 1.0f;
    gateHoldSamples = 0;
    gateIsOpen = true;
}

void TransientShaperDSP::setParameters(float attackVal, float sustainVal, int speedMode,
                                       float gateThreshDb, bool bypass) noexcept
{
    attackSmoothed.setCurrentAndTargetValue(juce::jlimit(-1.0f, 1.0f, attackVal));
    sustainSmoothed.setCurrentAndTargetValue(juce::jlimit(-1.0f, 1.0f, sustainVal));
    gateSmoothed.setCurrentAndTargetValue(juce::jlimit(-80.0f, 0.0f, gateThreshDb));

    if (speedParam != nullptr)
        speedParam->store(static_cast<float>(juce::jlimit(0, 1, speedMode)), std::memory_order_relaxed);
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void TransientShaperDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
        return;

    if (attackParam != nullptr)  attackSmoothed.setTargetValue(attackParam->load(std::memory_order_relaxed));
    if (sustainParam != nullptr) sustainSmoothed.setTargetValue(sustainParam->load(std::memory_order_relaxed));
    if (gateParam != nullptr)    gateSmoothed.setTargetValue(gateParam->load(std::memory_order_relaxed));

    const int speed = (speedParam != nullptr) ? static_cast<int>(speedParam->load(std::memory_order_relaxed)) : 0;
    const float modAttack  = ctx.getModulationOffset(ModTarget::DynTransAttack);
    const float modSustain = ctx.getModulationOffset(ModTarget::DynTransSustain);

    float* leftChannel  = ctx.mainBuffer.getWritePointer(0);
    float* rightChannel = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    float* auxLeft  = (ctx.hasAuxSplit && ctx.auxBandRejectBuffer.getNumChannels() > 0 && ctx.auxBandRejectBuffer.getNumSamples() >= numSamples)
                          ? ctx.auxBandRejectBuffer.getWritePointer(0) : nullptr;
    float* auxRight = (ctx.hasAuxSplit && ctx.auxBandRejectBuffer.getNumChannels() > 1 && ctx.auxBandRejectBuffer.getNumSamples() >= numSamples)
                          ? ctx.auxBandRejectBuffer.getWritePointer(1) : nullptr;

    // Fast envelope: 0.5 ms attack, 15 ms release
    const float fastAttAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.0005f));
    const float fastRelAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.015f));

    // Slow envelope: 15 ms attack, 140 ms (Fast) or 250 ms (Punch) release
    const float slowRelMs = (speed == 1) ? 0.250f : 0.140f;
    const float slowAttAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.015f));
    const float slowRelAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * slowRelMs));

    // Dedicated stereo-linked Gate detector: 0.2 ms peak attack, 40 ms release (bridges waveform zero-crossings cleanly)
    const float gateDetAttAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.0002f));
    const float gateDetRelAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.040f));

    // Gate gain ballistics: 1.0 ms open, 40 ms (Fast) or 75 ms (Punch) release close, plus 25 ms hold
    const float gateCloseSec = (speed == 1) ? 0.075f : 0.040f;
    const float gateOpenAlpha  = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.001f));
    const float gateCloseAlpha = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * gateCloseSec));
    const int   holdResetSamples = static_cast<int>(sampleRate * ((speed == 1) ? 0.035 : 0.020));

    constexpr int subBlockSize = 32;
    int samplesProcessed = 0;
    float minBlockGain = 1.0f;

    while (samplesProcessed < numSamples)
    {
        const int currentBlock = juce::jmin(subBlockSize, numSamples - samplesProcessed);

        const float curAttRaw  = attackSmoothed.skip(currentBlock);
        const float curSusRaw  = sustainSmoothed.skip(currentBlock);
        const float curGateDb  = gateSmoothed.skip(currentBlock);

        const float curAttack  = juce::jlimit(-1.0f, 1.0f, curAttRaw + modAttack);
        const float curSustain = juce::jlimit(-1.0f, 1.0f, curSusRaw + modSustain);

        // Map attack (-1..+1) to dynamic gain factor (up to +12 dB)
        const float attackFactor = curAttack * 3.98f; // ~ +12 dB boost when +1.0
        // Map sustain (-1..+1) to tail gain factor (-18 dB to +12 dB)
        const float sustainFactor = (curSustain >= 0.0f) ? (curSustain * 3.0f) : (curSustain * 0.85f);

        // Gate active when threshold is above -78 dB
        const bool gateActive = (curGateDb > -78.0f);
        const float gateOpenThreshLinear  = gateActive ? juce::Decibels::decibelsToGain(curGateDb) : 0.0f;
        const float gateCloseThreshLinear = gateOpenThreshLinear * 0.7079f; // -3.0 dB hysteresis margin

        for (int i = samplesProcessed; i < samplesProcessed + currentBlock; ++i)
        {
            float sL = leftChannel[i];
            float sR = (rightChannel != nullptr) ? rightChannel[i] : sL;

            const float absL = std::abs(sL);
            const float absR = std::abs(sR);

            // Full-spectrum detector peak: reconstructs full input (upper band + isolated sub-bass) so Sub Lock does not starve the gate detector
            const float fullInL = (auxLeft != nullptr)  ? (sL + auxLeft[i])  : sL;
            const float fullInR = (auxRight != nullptr) ? (sR + auxRight[i]) : sR;
            const float linkedPeak = std::max({ absL, absR, std::abs(fullInL), std::abs(fullInR) });

            // Left dual envelope tracking (true leaky integrator)
            envFastL += (absL > envFastL) ? (fastAttAlpha * (absL - envFastL))
                                          : (fastRelAlpha * (absL - envFastL));
            envSlowL += (absL > envSlowL) ? (slowAttAlpha * (absL - envSlowL))
                                          : (slowRelAlpha * (absL - envSlowL));

            // Right dual envelope tracking (true leaky integrator)
            envFastR += (absR > envFastR) ? (fastAttAlpha * (absR - envFastR))
                                          : (fastRelAlpha * (absR - envFastR));
            envSlowR += (absR > envSlowR) ? (slowAttAlpha * (absR - envSlowR))
                                          : (slowRelAlpha * (absR - envSlowR));

            // Stereo-linked gate envelope follower
            gateEnv += (linkedPeak > gateEnv) ? (gateDetAttAlpha * (linkedPeak - gateEnv))
                                              : (gateDetRelAlpha * (linkedPeak - gateEnv));

            // Differential calculation: positive = attack snap, negative = sustain tail
            const float diffL = (envFastL - envSlowL) / (envSlowL + 1e-4f);
            const float diffR = (envFastR - envSlowR) / (envSlowR + 1e-4f);

            float modGainL = 1.0f;
            if (diffL > 0.0f)
                modGainL += diffL * attackFactor;
            else
                modGainL += diffL * sustainFactor;

            float modGainR = 1.0f;
            if (diffR > 0.0f)
                modGainR += diffR * attackFactor;
            else
                modGainR += diffR * sustainFactor;

            modGainL = juce::jlimit(0.01f, 8.0f, modGainL);
            modGainR = juce::jlimit(0.01f, 8.0f, modGainR);

            sL *= modGainL;
            sR *= modGainR;

            float totalGainL = modGainL;
            float totalGainR = modGainR;

            // Stereo-linked Noise Gate Floor with Schmitt Trigger Hysteresis, Hold timer, and 2:1 Soft-Knee Expander
            if (gateActive)
            {
                if (gateEnv >= gateOpenThreshLinear)
                {
                    gateIsOpen = true;
                    gateHoldSamples = holdResetSamples;
                }
                else if (gateIsOpen)
                {
                    if (gateEnv >= gateCloseThreshLinear)
                    {
                        // Inside the -3 dB hysteresis zone while open: refresh hold
                        gateHoldSamples = holdResetSamples;
                    }
                    else if (gateHoldSamples > 0)
                    {
                        --gateHoldSamples;
                    }
                    else
                    {
                        gateIsOpen = false;
                    }
                }

                float targetGateGain = 1.0f;
                if (!gateIsOpen)
                {
                    // Progressive 2:1 downward expander curve below close threshold down to -80 dB floor
                    const float ratio = gateEnv / std::max(1e-6f, gateCloseThreshLinear);
                    const float clampedRatio = juce::jlimit(0.0f, 1.0f, ratio);
                    targetGateGain = clampedRatio * clampedRatio; // Smooth quadratic expansion (no binary chatter)
                }

                gateGain += (targetGateGain > gateGain) ? (gateOpenAlpha * (targetGateGain - gateGain))
                                                        : (gateCloseAlpha * (targetGateGain - gateGain));

                sL *= gateGain;
                sR *= gateGain;
                if (auxLeft != nullptr)  auxLeft[i]  *= gateGain;
                if (auxRight != nullptr) auxRight[i] *= gateGain;
                totalGainL *= gateGain;
                totalGainR *= gateGain;
            }
            else
            {
                // Smoothly restore full unity gain if gate parameter is disabled (far left)
                gateIsOpen = true;
                gateGain += gateOpenAlpha * (1.0f - gateGain);
            }

            minBlockGain = juce::jmin(minBlockGain, juce::jmin(totalGainL, totalGainR));

            leftChannel[i] = sL;
            if (rightChannel != nullptr)
                rightChannel[i] = sR;
        }

        samplesProcessed += currentBlock;
    }

    if (minBlockGain < 1.0f)
        ctx.cumulativeGainReductionDb += juce::Decibels::gainToDecibels(std::max(0.01f, minBlockGain));
}

void TransientShaperDSP::randomize(juce::Random& rng)
{
    if (attackSmoothed.isSmoothing())  attackSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 1.0f - 0.2f);
    if (sustainSmoothed.isSmoothing()) sustainSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 1.0f - 0.5f);
}

} // namespace wvzz::modules
