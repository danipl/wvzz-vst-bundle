#include "PhaserDSP.h"

namespace wvzz::modules {

PhaserDSP::PhaserDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void PhaserDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Phaser Bypass", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("rate"), 1 }, "Phaser Rate",
        juce::NormalisableRange<float>(0.02f, 10.0f, 0.01f, 0.4f), 0.5f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("depth"), 1 }, "Phaser Depth", 0.0f, 1.0f, 0.7f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("center"), 1 }, "Phaser Center",
        juce::NormalisableRange<float>(100.0f, 4000.0f, 1.0f, 0.4f), 800.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("stages"), 1 }, "Phaser Stages",
        juce::StringArray{ "2", "4", "6", "8", "12" }, 1));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("feedback"), 1 }, "Phaser Feedback",
        juce::NormalisableRange<float>(-0.95f, 0.95f, 0.01f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("stereo"), 1 }, "Phaser Stereo", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("mix"), 1 }, "Phaser Mix", 0.0f, 1.0f, 0.5f));
}

void PhaserDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam   = apvts.getRawParameterValue(paramId("bypass"));
    rateParam     = apvts.getRawParameterValue(paramId("rate"));
    depthParam    = apvts.getRawParameterValue(paramId("depth"));
    centerParam   = apvts.getRawParameterValue(paramId("center"));
    stagesParam   = apvts.getRawParameterValue(paramId("stages"));
    feedbackParam = apvts.getRawParameterValue(paramId("feedback"));
    stereoParam   = apvts.getRawParameterValue(paramId("stereo"));
    mixParam      = apvts.getRawParameterValue(paramId("mix"));
}

void PhaserDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const double rampTimeSec = 0.02;
    depthSmoothed.reset(sampleRate, rampTimeSec);
    centerSmoothed.reset(sampleRate, rampTimeSec);
    feedbackSmoothed.reset(sampleRate, rampTimeSec);
    mixSmoothed.reset(sampleRate, rampTimeSec);

    depthSmoothed.setCurrentAndTargetValue(depthParam != nullptr ? depthParam->load(std::memory_order_relaxed) : 0.7f);
    centerSmoothed.setCurrentAndTargetValue(centerParam != nullptr ? centerParam->load(std::memory_order_relaxed) : 800.0f);
    feedbackSmoothed.setCurrentAndTargetValue(feedbackParam != nullptr ? feedbackParam->load(std::memory_order_relaxed) : 0.5f);
    mixSmoothed.setCurrentAndTargetValue(mixParam != nullptr ? mixParam->load(std::memory_order_relaxed) : 0.5f);

    reset();
}

void PhaserDSP::reset() noexcept
{
    chainL.clear();
    chainR.clear();
    lfoPhase = 0.0;
}

float PhaserDSP::processChain(AllPassChain& chain, float input, float coeff, int stages, float fb) const noexcept
{
    float x = input + std::tanh(fb * chain.feedback);

    for (int s = 0; s < stages; ++s)
    {
        const auto idx = static_cast<size_t>(s);
        float y = coeff * x + chain.x1[idx] - coeff * chain.y1[idx];
        if (std::abs(y) < 1.0e-15f) // flush denormals in the recursive state
            y = 0.0f;
        chain.x1[idx] = x;
        chain.y1[idx] = y;
        x = y;
    }

    chain.feedback = x;
    return x;
}

void PhaserDSP::process(ProcessContext& ctx) noexcept
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

    const float rateRaw   = (rateParam != nullptr) ? rateParam->load(std::memory_order_relaxed) : 0.5f;
    const float depthRaw  = (depthParam != nullptr) ? depthParam->load(std::memory_order_relaxed) : 0.7f;
    const float centerRaw = (centerParam != nullptr) ? centerParam->load(std::memory_order_relaxed) : 800.0f;
    const float fbRaw     = (feedbackParam != nullptr) ? feedbackParam->load(std::memory_order_relaxed) : 0.5f;
    const float mixRaw    = (mixParam != nullptr) ? mixParam->load(std::memory_order_relaxed) : 0.5f;
    const float stereo    = (stereoParam != nullptr) ? stereoParam->load(std::memory_order_relaxed) : 0.5f;
    const int stages      = stagesForIndex(static_cast<int>((stagesParam != nullptr) ? stagesParam->load(std::memory_order_relaxed) : 1.0f));

    const float rate = juce::jlimit(0.02f, 10.0f, rateRaw * std::pow(2.0f, ctx.getModulationOffset(ModTarget::ModPhsRate) * 3.0f));
    depthSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, depthRaw));
    centerSmoothed.setTargetValue(juce::jlimit(100.0f, 4000.0f, centerRaw * std::pow(2.0f, ctx.getModulationOffset(ModTarget::ModPhsCenter) * 2.0f)));
    feedbackSmoothed.setTargetValue(juce::jlimit(-0.95f, 0.95f, fbRaw + ctx.getModulationOffset(ModTarget::ModPhsFeedback)));
    mixSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, mixRaw + ctx.getModulationOffset(ModTarget::ModPhsMix)));

    // Right channel LFO offset: stereo 1.0 = 180 degrees
    const double stereoOffset = static_cast<double>(stereo) * juce::MathConstants<double>::pi;
    lfoTap.publish(static_cast<float>(lfoPhase / juce::MathConstants<double>::twoPi), stereo * 0.5f,
                   depthSmoothed.getTargetValue(), dsp::LfoPhaseTap::Sine, true);

    float* left  = ctx.mainBuffer.getWritePointer(0);
    float* right = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    const double phaseIncrement = (juce::MathConstants<double>::twoPi * rate) / sampleRate;
    const float maxFreq = static_cast<float>(sampleRate * 0.45);
    const float piOverFs = juce::MathConstants<float>::pi / static_cast<float>(sampleRate);

    auto coeffFor = [maxFreq, piOverFs](float centerHz, float depth, float lfo) noexcept {
        const float fc = juce::jlimit(20.0f, maxFreq, centerHz * std::pow(2.0f, 2.0f * depth * lfo));
        const float t = std::tan(piOverFs * fc);
        return (t - 1.0f) / (t + 1.0f);
    };

    for (int i = 0; i < numSamples; ++i)
    {
        const float depth  = depthSmoothed.getNextValue();
        const float center = centerSmoothed.getNextValue();
        const float fb     = feedbackSmoothed.getNextValue();
        const float mix    = mixSmoothed.getNextValue();

        const float lfoL = static_cast<float>(std::sin(lfoPhase));
        const float inL = left[i];
        const float wetL = processChain(chainL, inL, coeffFor(center, depth, lfoL), stages, fb);
        left[i] = (1.0f - mix) * inL + mix * wetL;

        if (right != nullptr)
        {
            const float lfoR = static_cast<float>(std::sin(lfoPhase + stereoOffset));
            const float inR = right[i];
            const float wetR = processChain(chainR, inR, coeffFor(center, depth, lfoR), stages, fb);
            right[i] = (1.0f - mix) * inR + mix * wetR;
        }

        lfoPhase += phaseIncrement;
        if (lfoPhase >= juce::MathConstants<double>::twoPi)
            lfoPhase -= juce::MathConstants<double>::twoPi;
    }
}

void PhaserDSP::randomize(juce::Random& rng)
{
    juce::ignoreUnused(rng);
}

} // namespace wvzz::modules
