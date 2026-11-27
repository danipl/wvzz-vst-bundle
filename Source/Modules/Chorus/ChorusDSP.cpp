#include "ChorusDSP.h"

namespace wvzz::modules {

ChorusDSP::ChorusDSP(juce::String prefix)
    : instancePrefix(std::move(prefix))
{
}

void ChorusDSP::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ paramId("bypass"), 1 }, "Chorus Bypass", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("rate"), 1 }, "Chorus Rate",
        juce::NormalisableRange<float>(0.05f, 10.0f, 0.01f, 0.4f), 0.8f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("depth"), 1 }, "Chorus Depth", 0.0f, 1.0f, 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("delay"), 1 }, "Chorus Delay",
        juce::NormalisableRange<float>(2.0f, 30.0f, 0.1f), 12.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ paramId("voices"), 1 }, "Chorus Voices",
        juce::StringArray{ "1", "2", "3", "4" }, 2));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("spread"), 1 }, "Chorus Spread", 0.0f, 1.0f, 0.7f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("feedback"), 1 }, "Chorus Feedback", 0.0f, 0.7f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ paramId("mix"), 1 }, "Chorus Mix", 0.0f, 1.0f, 0.5f));
}

void ChorusDSP::bindParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam   = apvts.getRawParameterValue(paramId("bypass"));
    rateParam     = apvts.getRawParameterValue(paramId("rate"));
    depthParam    = apvts.getRawParameterValue(paramId("depth"));
    delayParam    = apvts.getRawParameterValue(paramId("delay"));
    voicesParam   = apvts.getRawParameterValue(paramId("voices"));
    spreadParam   = apvts.getRawParameterValue(paramId("spread"));
    feedbackParam = apvts.getRawParameterValue(paramId("feedback"));
    mixParam      = apvts.getRawParameterValue(paramId("mix"));
}

void ChorusDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    bufferSize = static_cast<int>(sampleRate * 0.05) + 16; // 30 ms delay + 5 ms sweep + margin
    bufferL.assign(static_cast<size_t>(bufferSize), 0.0f);
    bufferR.assign(static_cast<size_t>(bufferSize), 0.0f);

    const double rampTimeSec = 0.02;
    depthSmoothed.reset(sampleRate, rampTimeSec);
    delaySmoothed.reset(sampleRate, 0.05);
    mixSmoothed.reset(sampleRate, rampTimeSec);

    depthSmoothed.setCurrentAndTargetValue(depthParam != nullptr ? depthParam->load(std::memory_order_relaxed) : 0.5f);
    delaySmoothed.setCurrentAndTargetValue(delayParam != nullptr ? delayParam->load(std::memory_order_relaxed) : 12.0f);
    mixSmoothed.setCurrentAndTargetValue(mixParam != nullptr ? mixParam->load(std::memory_order_relaxed) : 0.5f);

    reset();
}

void ChorusDSP::reset() noexcept
{
    std::fill(bufferL.begin(), bufferL.end(), 0.0f);
    std::fill(bufferR.begin(), bufferR.end(), 0.0f);
    writeIndex = 0;
    lfoPhase = 0.0;
    feedbackL = 0.0f;
    feedbackR = 0.0f;
}

float ChorusDSP::readLagrange(const std::vector<float>& buf, float delay) const noexcept
{
    float readPos = static_cast<float>(writeIndex) - delay;
    while (readPos < 0.0f)
        readPos += static_cast<float>(bufferSize);

    int i1 = static_cast<int>(readPos);
    if (i1 >= bufferSize)
        i1 -= bufferSize;
    const int i0 = (i1 - 1 + bufferSize) % bufferSize;
    const int i2 = (i1 + 1) % bufferSize;
    const int i3 = (i1 + 2) % bufferSize;

    const float d = readPos - std::floor(readPos);

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

void ChorusDSP::process(ProcessContext& ctx) noexcept
{
    const int numSamples = ctx.mainBuffer.getNumSamples();
    const int numChannels = juce::jmin(2, ctx.mainBuffer.getNumChannels());

    if (numSamples == 0 || numChannels == 0 || bufferSize <= 0)
        return;

    const bool bypassed = (bypassParam != nullptr) && (bypassParam->load(std::memory_order_relaxed) > 0.5f);
    if (bypassed)
    {
        lfoTap.setActive(false);
        return;
    }

    const float rateRaw  = (rateParam != nullptr) ? rateParam->load(std::memory_order_relaxed) : 0.8f;
    const float depthRaw = (depthParam != nullptr) ? depthParam->load(std::memory_order_relaxed) : 0.5f;
    const float mixRaw   = (mixParam != nullptr) ? mixParam->load(std::memory_order_relaxed) : 0.5f;
    const float spread   = (spreadParam != nullptr) ? spreadParam->load(std::memory_order_relaxed) : 0.7f;
    const float feedback = juce::jlimit(0.0f, 0.7f, (feedbackParam != nullptr) ? feedbackParam->load(std::memory_order_relaxed) : 0.0f);
    const int numVoices  = juce::jlimit(1, maxVoices, 1 + static_cast<int>((voicesParam != nullptr) ? voicesParam->load(std::memory_order_relaxed) : 2.0f));

    const float rate = juce::jlimit(0.05f, 10.0f, rateRaw * std::pow(2.0f, ctx.getModulationOffset(ModTarget::ModChorRate) * 3.0f));
    depthSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, depthRaw + ctx.getModulationOffset(ModTarget::ModChorDepth)));
    mixSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, mixRaw + ctx.getModulationOffset(ModTarget::ModChorMix)));
    if (delayParam != nullptr)
        delaySmoothed.setTargetValue(delayParam->load(std::memory_order_relaxed));

    // Right channel LFO offset: spread 1.0 = 90 degrees
    const double stereoOffset = static_cast<double>(spread) * juce::MathConstants<double>::halfPi;
    lfoTap.publish(static_cast<float>(lfoPhase / juce::MathConstants<double>::twoPi), spread * 0.25f,
                   depthSmoothed.getTargetValue(), dsp::LfoPhaseTap::Sine, true);

    float* left  = ctx.mainBuffer.getWritePointer(0);
    float* right = (numChannels > 1) ? ctx.mainBuffer.getWritePointer(1) : nullptr;

    const double phaseIncrement = (juce::MathConstants<double>::twoPi * rate) / sampleRate;
    const double voicePhaseStep = juce::MathConstants<double>::twoPi / static_cast<double>(numVoices);
    const float voiceGain = 1.0f / std::sqrt(static_cast<float>(numVoices));
    const float msToSamples = static_cast<float>(sampleRate * 0.001);
    const float maxDelay = static_cast<float>(bufferSize - 4);

    for (int i = 0; i < numSamples; ++i)
    {
        const float depth = depthSmoothed.getNextValue();
        const float mix   = mixSmoothed.getNextValue();
        const float baseDelay = delaySmoothed.getNextValue() * msToSamples;
        const float sweep = depth * maxSweepMs * msToSamples;

        const float inL = left[i];
        const float inR = (right != nullptr) ? right[i] : inL;

        float wetL = 0.0f;
        float wetR = 0.0f;
        float firstVoiceL = 0.0f;
        float firstVoiceR = 0.0f;

        for (int v = 0; v < numVoices; ++v)
        {
            const double voicePhase = lfoPhase + voicePhaseStep * static_cast<double>(v);
            const float lfoL = static_cast<float>(std::sin(voicePhase));
            const float lfoR = static_cast<float>(std::sin(voicePhase + stereoOffset));

            const float tapL = readLagrange(bufferL, juce::jlimit(1.0f, maxDelay, baseDelay + sweep * lfoL));
            const float tapR = readLagrange(bufferR, juce::jlimit(1.0f, maxDelay, baseDelay + sweep * lfoR));

            if (v == 0)
            {
                firstVoiceL = tapL;
                firstVoiceR = tapR;
            }
            wetL += tapL;
            wetR += tapR;
        }

        wetL *= voiceGain;
        wetR *= voiceGain;

        // Feedback from the first voice, soft-limited; flush denormals in the loop
        feedbackL = (std::abs(firstVoiceL) < 1.0e-15f) ? 0.0f : firstVoiceL;
        feedbackR = (std::abs(firstVoiceR) < 1.0e-15f) ? 0.0f : firstVoiceR;
        bufferL[static_cast<size_t>(writeIndex)] = inL + std::tanh(feedback * feedbackL);
        bufferR[static_cast<size_t>(writeIndex)] = inR + std::tanh(feedback * feedbackR);

        left[i] = (1.0f - mix) * inL + mix * wetL;
        if (right != nullptr)
            right[i] = (1.0f - mix) * inR + mix * wetR;

        if (++writeIndex >= bufferSize)
            writeIndex = 0;

        lfoPhase += phaseIncrement;
        if (lfoPhase >= juce::MathConstants<double>::twoPi)
            lfoPhase -= juce::MathConstants<double>::twoPi;
    }
}

void ChorusDSP::randomize(juce::Random& rng)
{
    juce::ignoreUnused(rng);
}

} // namespace wvzz::modules
