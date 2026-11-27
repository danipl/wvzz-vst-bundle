#include "MaximizerClipperDSP.h"

namespace wvzz::modules {

MaximizerClipperDSP::MaximizerClipperDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void MaximizerClipperDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Clipper Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("drive"), 1 }, "Clipper Drive", 0.0f, 18.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("ceiling"), 1 }, "Clipper Ceiling", -12.0f, 0.0f, -0.1f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("mode"), 1 }, "Clipper Mode", getClipperModeNames(), 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("mix"), 1 }, "Clipper Mix", 0.0f, 1.0f, 1.0f));
}

void MaximizerClipperDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam  = apvts.getRawParameterValue(paramId("bypass"));
    driveParam   = apvts.getRawParameterValue(paramId("drive"));
    ceilingParam = apvts.getRawParameterValue(paramId("ceiling"));
    modeParam    = apvts.getRawParameterValue(paramId("mode"));
    mixParam     = apvts.getRawParameterValue(paramId("mix"));
}

void MaximizerClipperDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const double rampTimeSec = 0.015;
    driveSmoothed.reset(sampleRate, rampTimeSec);
    ceilingSmoothed.reset(sampleRate, rampTimeSec);
    mixSmoothed.reset(sampleRate, rampTimeSec);

    driveSmoothed.setCurrentAndTargetValue(driveParam != nullptr ? driveParam->load(std::memory_order_relaxed) : 0.0f);
    ceilingSmoothed.setCurrentAndTargetValue(ceilingParam != nullptr ? ceilingParam->load(std::memory_order_relaxed) : -0.1f);
    mixSmoothed.setCurrentAndTargetValue(mixParam != nullptr ? mixParam->load(std::memory_order_relaxed) : 1.0f);

    reset();
}

void MaximizerClipperDSP::reset() noexcept
{
    prevInL = 0.0f;
    prevInR = 0.0f;
}

void MaximizerClipperDSP::setParameters(float driveDb, float ceilingDb, int mode,
                                        float mixNorm, bool bypass) noexcept
{
    modeFallback = mode;
    bypassFallback = bypass;

    driveSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 18.0f, driveDb));
    ceilingSmoothed.setCurrentAndTargetValue(juce::jlimit(-12.0f, 0.0f, ceilingDb));
    mixSmoothed.setCurrentAndTargetValue(juce::jlimit(0.0f, 1.0f, mixNorm));

    if (modeParam != nullptr)
        modeParam->store(static_cast<float>(juce::jlimit(0, 2, mode)), std::memory_order_relaxed);
    if (bypassParam != nullptr)
        bypassParam->store(bypass ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void MaximizerClipperDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) ? (bypassParam->load(std::memory_order_relaxed) > 0.5f) : bypassFallback;
    if (bypassed)
        return;

    if (driveParam != nullptr)   driveSmoothed.setTargetValue(driveParam->load(std::memory_order_relaxed));
    if (ceilingParam != nullptr) ceilingSmoothed.setTargetValue(ceilingParam->load(std::memory_order_relaxed));
    if (mixParam != nullptr)     mixSmoothed.setTargetValue(mixParam->load(std::memory_order_relaxed));

    const int mode = (modeParam != nullptr) ? static_cast<int>(modeParam->load(std::memory_order_relaxed)) : modeFallback;
    const float modDrive = ctx.getModulationOffset(ModTarget::DynClipDrive);

    float* leftChannel  = ctx.mainBuffer.getWritePointer(0);
    float* rightChannel = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    constexpr int subBlockSize = 32;
    int samplesProcessed = 0;

    while (samplesProcessed < numSamples)
    {
        const int currentBlock = juce::jmin(subBlockSize, numSamples - samplesProcessed);

        const float rawDriveDb = driveSmoothed.skip(currentBlock);
        const float curCeilingDb = ceilingSmoothed.skip(currentBlock);
        const float curMix = mixSmoothed.skip(currentBlock);

        const float curDriveDb = juce::jlimit(0.0f, 24.0f, rawDriveDb + modDrive * 18.0f);
        const float driveGain = juce::Decibels::decibelsToGain(curDriveDb);
        const float ceilingLinear = juce::Decibels::decibelsToGain(curCeilingDb);
        const float invCeiling = 1.0f / std::max(1e-4f, ceilingLinear);

        for (int i = samplesProcessed; i < samplesProcessed + currentBlock; ++i)
        {
            const float inL = leftChannel[i];
            const float inR = (rightChannel != nullptr) ? rightChannel[i] : inL;

            const float drivenL = inL * driveGain;
            const float drivenR = inR * driveGain;

            const float uL = drivenL * invCeiling;
            const float uR = drivenR * invCeiling;

            const float clippedL = processADAA(uL, prevInL, mode) * ceilingLinear;
            const float clippedR = processADAA(uR, prevInR, mode) * ceilingLinear;

            leftChannel[i] = inL * (1.0f - curMix) + clippedL * curMix;
            if (rightChannel != nullptr)
                rightChannel[i] = inR * (1.0f - curMix) + clippedR * curMix;
        }

        samplesProcessed += currentBlock;
    }
}

void MaximizerClipperDSP::randomize(juce::Random& rng)
{
    if (driveSmoothed.isSmoothing()) driveSmoothed.setCurrentAndTargetValue(rng.nextFloat() * 12.0f);
}

} // namespace wvzz::modules
