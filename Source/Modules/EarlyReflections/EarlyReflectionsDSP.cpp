#include "EarlyReflectionsDSP.h"

namespace wvzz::modules {

namespace {
    // Tap positions as a fraction of the pattern length, and tap gains. Left and right differ,
    // so the two sides are decorrelated. Gains fall with time; mixed signs avoid a comb-filter tone.
    constexpr float tapPosition[2][EarlyReflectionsDSP::numTaps] = {
        { 0.113f, 0.237f, 0.331f, 0.467f, 0.563f, 0.709f, 0.853f, 1.000f },
        { 0.089f, 0.199f, 0.367f, 0.431f, 0.607f, 0.743f, 0.811f, 0.967f }
    };
    constexpr float tapGain[2][EarlyReflectionsDSP::numTaps] = {
        { 0.85f, -0.72f, 0.63f, 0.55f, -0.47f, 0.40f, -0.33f, 0.27f },
        { 0.88f, 0.69f, -0.60f, 0.57f, 0.45f, -0.41f, 0.31f, -0.28f }
    };
    constexpr float diffuserMs[2][2] = { { 2.3f, 3.7f }, { 2.9f, 3.1f } };
}

EarlyReflectionsDSP::EarlyReflectionsDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void EarlyReflectionsDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Early Reflections Bypass", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("size"), 1 }, "Early Reflections Size", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("diffusion"), 1 }, "Early Reflections Diffusion", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("level"), 1 }, "Early Reflections Level", 0.0f, 1.0f, 0.5f));
}

void EarlyReflectionsDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam    = apvts.getRawParameterValue(paramId("bypass"));
    sizeParam      = apvts.getRawParameterValue(paramId("size"));
    diffusionParam = apvts.getRawParameterValue(paramId("diffusion"));
    levelParam     = apvts.getRawParameterValue(paramId("level"));
}

float EarlyReflectionsDSP::getPatternMs() const noexcept
{
    const float size = (sizeParam != nullptr) ? sizeParam->load(std::memory_order_relaxed) : 0.5f;
    return minPatternMs + (maxPatternMs - minPatternMs) * juce::jlimit(0.0f, 1.0f, size);
}

bool EarlyReflectionsDSP::isActive() const noexcept
{
    return !((bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f));
}

void EarlyReflectionsDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    for (size_t c = 0; c < channels.size(); ++c)
    {
        channels[c].line.prepare(static_cast<int>(sampleRate * 0.12));
        for (size_t d = 0; d < channels[c].diffusers.size(); ++d)
            channels[c].diffusers[d].prepare(static_cast<int>(sampleRate * diffuserMs[c][d] * 0.001));
    }

    levelSmoothed.reset(sampleRate, 0.02);
    sizeSmoothed.reset(sampleRate, 0.1);
    levelSmoothed.setCurrentAndTargetValue(isActive() ? (levelParam != nullptr ? levelParam->load(std::memory_order_relaxed) : 0.5f) : 0.0f);
    sizeSmoothed.setCurrentAndTargetValue(getPatternMs());

    reset();
}

void EarlyReflectionsDSP::reset() noexcept
{
    for (auto& ch : channels)
    {
        if (ch.line.isPrepared())
            ch.line.clear();
        for (auto& d : ch.diffusers)
            d.clear();
    }
}

void EarlyReflectionsDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0 || !channels[0].line.isPrepared())
        return;
    if (ctx.auxBandRejectBuffer.getNumChannels() < 2 || ctx.auxBandRejectBuffer.getNumSamples() < numSamples)
        return;

    // Bypass fades the level to 0 first (no click). Once it is silent the bus is left alone.
    const bool active = isActive();
    if (!active && !levelSmoothed.isSmoothing() && levelSmoothed.getCurrentValue() <= 0.0f)
    {
        // Keep the lines running on silence so a later un-bypass starts clean
        for (int i = 0; i < numSamples; ++i)
        {
            channels[0].line.push(0.0f);
            channels[1].line.push(0.0f);
        }
        return;
    }

    const float levelRaw  = (levelParam != nullptr) ? levelParam->load(std::memory_order_relaxed) : 0.5f;
    const float diffusion = 0.6f * juce::jlimit(0.0f, 1.0f, (diffusionParam != nullptr) ? diffusionParam->load(std::memory_order_relaxed) : 0.5f);

    levelSmoothed.setTargetValue(active ? juce::jlimit(0.0f, 1.0f, levelRaw + ctx.getModulationOffset(ModTarget::ErLevel)) : 0.0f);
    sizeSmoothed.setTargetValue(getPatternMs());

    const float* inL = ctx.mainBuffer.getReadPointer(0);
    const float* inR = (numChannels > 1) ? ctx.mainBuffer.getReadPointer(1) : inL;
    float* auxL = ctx.auxBandRejectBuffer.getWritePointer(0);
    float* auxR = ctx.auxBandRejectBuffer.getWritePointer(1);

    const float msToSamples = static_cast<float>(sampleRate) * 0.001f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float level = levelSmoothed.getNextValue();
        const float patternSamples = sizeSmoothed.getNextValue() * msToSamples;

        float out[2] = { 0.0f, 0.0f };
        for (size_t c = 0; c < 2; ++c)
        {
            auto& ch = channels[c];
            float sum = 0.0f;
            for (int t = 0; t < numTaps; ++t)
                sum += tapGain[c][t] * ch.line.read(tapPosition[c][t] * patternSamples);

            for (auto& d : ch.diffusers)
                sum = d.process(sum, diffusion);

            out[c] = sum * 0.5f * level; // 0.5: the 8 taps together are about as loud as the input
        }

        channels[0].line.push(inL[i]);
        channels[1].line.push(inR[i]);

        auxL[i] = out[0];
        auxR[i] = out[1];
    }

    ctx.hasAuxSplit = true;
}

void EarlyReflectionsDSP::randomize(juce::Random& rng)
{
    juce::ignoreUnused(rng);
}

} // namespace wvzz::modules
