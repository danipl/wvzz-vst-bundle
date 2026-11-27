#include "GrainDSP.h"

namespace wvzz::modules {

GrainDSP::GrainDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void GrainDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("size"), 1 }, "Grain Size",
        juce::NormalisableRange<float>(10.0f, 500.0f, 0.1f, 0.5f), 80.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("density"), 1 }, "Grain Density",
        juce::NormalisableRange<float>(2.0f, 60.0f, 0.1f, 0.5f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("pitch"), 1 }, "Grain Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("st")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("spray"), 1 }, "Grain Spray", 0.0f, 1.0f, 0.1f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("reverse"), 1 }, "Grain Reverse", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("spread"), 1 }, "Grain Spread", 0.0f, 1.0f, 0.5f));
}

void GrainDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    sizeParam    = apvts.getRawParameterValue(paramId("size"));
    densityParam = apvts.getRawParameterValue(paramId("density"));
    pitchParam   = apvts.getRawParameterValue(paramId("pitch"));
    sprayParam   = apvts.getRawParameterValue(paramId("spray"));
    reverseParam = apvts.getRawParameterValue(paramId("reverse"));
    spreadParam  = apvts.getRawParameterValue(paramId("spread"));
}

void GrainDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    engine.prepare(sampleRate);

    if (!externalHost)
    {
        const int oneSecond = static_cast<int>(sampleRate);
        lineL.prepare(oneSecond);
        lineR.prepare(oneSecond);
    }

    settings = {};
    settings.sizeSamples = static_cast<float>(sampleRate * 0.08);
}

void GrainDSP::reset() noexcept
{
    engine.reset();
    if (lineL.isPrepared()) lineL.clear();
    if (lineR.isPrepared()) lineR.clear();
}

void GrainDSP::updateBlock(const ProcessContext& ctx) noexcept
{
    const float sizeMs  = (sizeParam != nullptr) ? sizeParam->load(std::memory_order_relaxed) : 80.0f;
    const float density = (densityParam != nullptr) ? densityParam->load(std::memory_order_relaxed) : 20.0f;
    const float pitchSt = (pitchParam != nullptr) ? pitchParam->load(std::memory_order_relaxed) : 0.0f;
    const float spray   = (sprayParam != nullptr) ? sprayParam->load(std::memory_order_relaxed) : 0.1f;
    const float reverse = (reverseParam != nullptr) ? reverseParam->load(std::memory_order_relaxed) : 0.0f;
    const float spread  = (spreadParam != nullptr) ? spreadParam->load(std::memory_order_relaxed) : 0.5f;

    const float modSizeMs  = juce::jlimit(10.0f, 500.0f, sizeMs * std::pow(2.0f, ctx.getModulationOffset(ModTarget::GrainSize) * 2.0f));
    const float modPitchSt = juce::jlimit(-24.0f, 24.0f, pitchSt + ctx.getModulationOffset(ModTarget::GrainPitch) * 12.0f);
    const float modSpray   = juce::jlimit(0.0f, 1.0f, spray + ctx.getModulationOffset(ModTarget::GrainSpray));

    settings.sizeSamples     = static_cast<float>(sampleRate) * modSizeMs * 0.001f;
    settings.grainsPerSecond = juce::jlimit(2.0f, 60.0f, density);
    settings.pitchRatio      = std::pow(2.0f, modPitchSt / 12.0f);
    settings.spraySamples    = modSpray * static_cast<float>(sampleRate) * 0.2f; // up to 200 ms
    settings.reverseChance   = juce::jlimit(0.0f, 1.0f, reverse);
    settings.stereoSpread    = juce::jlimit(0.0f, 1.0f, spread);
}

void GrainDSP::process(ProcessContext& ctx) noexcept
{
    // Hosted use: the owner of the delay lines calls updateBlock() and renderGrains().
    if (externalHost || !lineL.isPrepared())
        return;

    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());
    if (numSamples == 0 || numChannels == 0)
        return;

    updateBlock(ctx);

    float* left  = ctx.mainBuffer.getWritePointer(0);
    float* right = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;
    const float baseDelay = static_cast<float>(sampleRate * 0.1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = left[i];
        const float inR = (right != nullptr) ? right[i] : inL;

        float outL = 0.0f, outR = 0.0f;
        engine.process(lineL, lineR, baseDelay, baseDelay, settings, outL, outR);

        lineL.push(inL);
        lineR.push(inR);

        left[i] = outL;
        if (right != nullptr)
            right[i] = outR;
    }
}

void GrainDSP::randomize(juce::Random& rng)
{
    juce::ignoreUnused(rng);
}

} // namespace wvzz::modules
