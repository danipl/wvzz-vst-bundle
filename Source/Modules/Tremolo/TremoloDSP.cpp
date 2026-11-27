#include "TremoloDSP.h"

namespace wvzz::modules {

TremoloDSP::TremoloDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void TremoloDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Tremolo Bypass", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("rate"), 1 }, "Tremolo Rate",
        juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f, 0.5f), 4.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("depth"), 1 }, "Tremolo Depth", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("shape"), 1 }, "Tremolo Shape",
        juce::StringArray{ "Sine", "Triangle", "Square" }, 0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("mode"), 1 }, "Tremolo Mode",
        juce::StringArray{ "Classic", "Harmonic" }, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("stereo"), 1 }, "Tremolo Stereo", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("xover"), 1 }, "Tremolo Crossover",
        juce::NormalisableRange<float>(200.0f, 2000.0f, 1.0f, 0.5f), 800.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
}

void TremoloDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam = apvts.getRawParameterValue(paramId("bypass"));
    rateParam   = apvts.getRawParameterValue(paramId("rate"));
    depthParam  = apvts.getRawParameterValue(paramId("depth"));
    shapeParam  = apvts.getRawParameterValue(paramId("shape"));
    modeParam   = apvts.getRawParameterValue(paramId("mode"));
    stereoParam = apvts.getRawParameterValue(paramId("stereo"));
    xoverParam  = apvts.getRawParameterValue(paramId("xover"));
}

void TremoloDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    depthSmoothed.reset(sampleRate, 0.02);
    depthSmoothed.setCurrentAndTargetValue(depthParam != nullptr ? depthParam->load(std::memory_order_relaxed) : 0.5f);
    lfoSmoothCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.002f));
    reset();
}

void TremoloDSP::reset() noexcept
{
    lfoPhase = 0.0;
    lfoStateL = 0.0f;
    lfoStateR = 0.0f;
    lowStateL = 0.0f;
    lowStateR = 0.0f;
}

float TremoloDSP::evaluate(double phase, int shape) noexcept
{
    const double twoPi = juce::MathConstants<double>::twoPi;
    double p = std::fmod(phase, twoPi);
    if (p < 0.0)
        p += twoPi;
    const float n = static_cast<float>(p / twoPi); // 0..1

    switch (shape)
    {
        case Triangle: return (n < 0.5f) ? (4.0f * n - 1.0f) : (3.0f - 4.0f * n);
        case Square:   return (n < 0.5f) ? 1.0f : -1.0f;
        default:       return static_cast<float>(std::sin(p));
    }
}

void TremoloDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
    {
        lfoTap.setActive(false);
        return;
    }

    const float rateRaw  = (rateParam != nullptr) ? rateParam->load(std::memory_order_relaxed) : 4.0f;
    const float depthRaw = (depthParam != nullptr) ? depthParam->load(std::memory_order_relaxed) : 0.5f;
    const int shape      = static_cast<int>((shapeParam != nullptr) ? shapeParam->load(std::memory_order_relaxed) : 0.0f);
    const int mode       = static_cast<int>((modeParam != nullptr) ? modeParam->load(std::memory_order_relaxed) : 0.0f);
    const float stereo   = (stereoParam != nullptr) ? stereoParam->load(std::memory_order_relaxed) : 0.0f;
    const float xoverHz  = juce::jlimit(200.0f, 2000.0f, (xoverParam != nullptr) ? xoverParam->load(std::memory_order_relaxed) : 800.0f);

    const float rate = juce::jlimit(0.1f, 20.0f, rateRaw * std::pow(2.0f, ctx.getModulationOffset(ModTarget::ModTremRate) * 3.0f));
    depthSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, depthRaw + ctx.getModulationOffset(ModTarget::ModTremDepth)));

    // Right channel LFO offset: stereo 1.0 = 180 degrees
    const double stereoOffset = static_cast<double>(stereo) * juce::MathConstants<double>::pi;
    lfoTap.publish(static_cast<float>(lfoPhase / juce::MathConstants<double>::twoPi), stereo * 0.5f,
                   depthSmoothed.getTargetValue(), shape, true);

    float* left  = ctx.mainBuffer.getWritePointer(0);
    float* right = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    const double phaseIncrement = (juce::MathConstants<double>::twoPi * rate) / sampleRate;
    const float lowCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * xoverHz / static_cast<float>(sampleRate));
    // Sine and triangle are already smooth; only the square needs the 2 ms smoothing
    const float smooth = (shape == Square) ? lfoSmoothCoeff : 1.0f;

    auto processSample = [mode, lowCoeff](float in, float lfo, float depth, float& lowState) noexcept {
        if (mode == Harmonic)
        {
            lowState += lowCoeff * (in - lowState);
            if (std::abs(lowState) < 1.0e-15f)
                lowState = 0.0f;
            const float low = lowState;
            const float high = in - low; // low + high == in
            const float gainLow  = 1.0f - depth * 0.5f * (1.0f - lfo);
            const float gainHigh = 1.0f - depth * 0.5f * (1.0f + lfo);
            return low * gainLow + high * gainHigh;
        }
        return in * (1.0f - depth * 0.5f * (1.0f - lfo));
    };

    for (int i = 0; i < numSamples; ++i)
    {
        const float depth = depthSmoothed.getNextValue();

        lfoStateL += smooth * (evaluate(lfoPhase, shape) - lfoStateL);
        left[i] = processSample(left[i], lfoStateL, depth, lowStateL);

        if (right != nullptr)
        {
            lfoStateR += smooth * (evaluate(lfoPhase + stereoOffset, shape) - lfoStateR);
            right[i] = processSample(right[i], lfoStateR, depth, lowStateR);
        }

        lfoPhase += phaseIncrement;
        if (lfoPhase >= juce::MathConstants<double>::twoPi)
            lfoPhase -= juce::MathConstants<double>::twoPi;
    }
}

void TremoloDSP::randomize(juce::Random& rng)
{
    juce::ignoreUnused(rng);
}

} // namespace wvzz::modules
