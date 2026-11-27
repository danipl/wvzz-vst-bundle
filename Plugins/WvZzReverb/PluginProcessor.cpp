#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Core/UI/ReverbDecayDisplayComponent.h"

namespace wvzz {

WvZzReverbAudioProcessor::WvZzReverbAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      earlyModule("er"),
      reverbModule("rev"),
      modModule("mod", &visualizerSource, ModProfile::Reverb),
      masterModule("master"),
      apvts(*this, nullptr, "Parameters",
            createAggregatedLayout(earlyModule, reverbModule, modModule, masterModule))
{
    earlyModule.getDSP().bindParameters(apvts);
    reverbModule.getDSP().bindParameters(apvts);
    modModule.getDSP().bindParameters(apvts);
    masterModule.getDSP().bindParameters(apvts);

    // Custom center tab: decay-curve display. It reads only the lock-free ReverbDecayState.
    modModule.setCustomDisplay("DECAY", [this]() -> std::unique_ptr<juce::Component> {
        return std::make_unique<ui::ReverbDecayDisplayComponent>(reverbModule.getReverbDSP().getDecayState());
    });

    // Start on the first factory preset.
    setCurrentProgram(0);
}

WvZzReverbAudioProcessor::~WvZzReverbAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout WvZzReverbAudioProcessor::createAggregatedLayout(
    modules::EarlyReflectionsModule& early,
    modules::ReverbModule& reverb,
    modules::ModulationModule& mod,
    modules::MasterModule& master)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    early.getDSP().registerParameters(layout);
    reverb.getDSP().registerParameters(layout);
    mod.getDSP().registerParameters(layout);
    master.getDSP().registerParameters(layout);
    return layout;
}

const juce::String WvZzReverbAudioProcessor::getName() const
{
    return "WvZz Reverb";
}

bool WvZzReverbAudioProcessor::acceptsMidi() const { return true; }
bool WvZzReverbAudioProcessor::producesMidi() const { return false; }
bool WvZzReverbAudioProcessor::isMidiEffect() const { return false; }

double WvZzReverbAudioProcessor::getTailLengthSeconds() const
{
    // Pre-delay + decay time (60 dB) + the early-reflections pattern
    return reverbModule.getReverbDSP().getTailSeconds() + 0.1;
}

int WvZzReverbAudioProcessor::getNumPrograms() { return static_cast<int>(reverbfx::getFactoryPresets().size()); }
int WvZzReverbAudioProcessor::getCurrentProgram() { return currentProgramIndex; }

void WvZzReverbAudioProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return;

    currentProgramIndex = index;
    const auto& preset = reverbfx::getFactoryPresets()[static_cast<size_t>(index)];

    // 1. Every parameter back to its default, so the result does not depend on the previous preset.
    for (auto* param : getParameters())
    {
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
            ranged->setValueNotifyingHost(ranged->getDefaultValue());
    }

    // 2. Apply the preset values.
    for (const auto& [paramId, val] : preset.values)
    {
        if (auto* param = apvts.getParameter(paramId))
            param->setValueNotifyingHost(param->convertTo0to1(val));
    }
}

const juce::String WvZzReverbAudioProcessor::getProgramName(int index)
{
    const auto& presets = reverbfx::getFactoryPresets();
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[static_cast<size_t>(index)].name;
    return {};
}

void WvZzReverbAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void WvZzReverbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec {
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        2
    };

    visualizerSource.prepare(sampleRate, samplesPerBlock);

    earlyModule.getDSP().prepare(spec);
    reverbModule.getDSP().prepare(spec);
    modModule.getDSP().prepare(spec);
    masterModule.getDSP().prepare(spec);

    dryInputBuffer.setSize(2, samplesPerBlock, false, false, true);
    auxBuffer.setSize(2, samplesPerBlock, false, false, true);
}

void WvZzReverbAudioProcessor::releaseResources()
{
    earlyModule.getDSP().reset();
    reverbModule.getDSP().reset();
    modModule.getDSP().reset();
    masterModule.getDSP().reset();
    visualizerSource.reset();
}

bool WvZzReverbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void WvZzReverbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) noexcept
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0)
        return;

    // A host may send a larger block than announced. The scratch buffers were sized in
    // prepareToPlay; skip processing instead of allocating on the audio thread.
    if (dryInputBuffer.getNumSamples() < numSamples || auxBuffer.getNumSamples() < numSamples)
        return;

    // 1. Input gain, then keep the dry signal for the Master wet/dry mix and bypass
    masterModule.getMasterDSP().applyInputGain(buffer);

    const int numMainChannels = juce::jmin(2, buffer.getNumChannels());
    for (int ch = 0; ch < numMainChannels; ++ch)
        dryInputBuffer.copyFrom(ch, 0, buffer.getReadPointer(ch), numSamples);
    if (numMainChannels == 1)
        dryInputBuffer.copyFrom(1, 0, buffer.getReadPointer(0), numSamples);

    visualizerSource.pushInput(buffer.getArrayOfReadPointers(), buffer.getNumChannels(), numSamples);

    // 2. ProcessContext: the "sidechain" slot carries the dry signal for the Master module
    bool hasAuxSplit = false;
    ProcessContext ctx {
        buffer,
        dryInputBuffer,
        auxBuffer,
        hasAuxSplit,
        nullptr,
        0,
        120.0,
        0.0,
        false,
        0,
        0.0f
    };

    if (auto* playHead = getPlayHead())
    {
        if (auto pos = playHead->getPosition())
        {
            if (pos->getBpm().hasValue())
                ctx.hostBpm = *pos->getBpm();
            if (pos->getPpqPosition().hasValue())
                ctx.hostPpqPosition = *pos->getPpqPosition();
            ctx.isHostPlaying = pos->getIsPlaying();
        }
    }

    // 3. LFO 1 / LFO 2 / X-Y pad offsets for this block
    modModule.getDSP().process(ctx);

    // 4. Early reflections write to the auxiliary bus; the reverb uses it as extra input and wet signal.
    auto& earlyDSP = earlyModule.getEarlyReflectionsDSP();
    reverbModule.getReverbDSP().setEarlyPatternMs(earlyDSP.isActive() ? earlyDSP.getPatternMs() : 0.0f);
    earlyDSP.process(ctx);
    reverbModule.getDSP().process(ctx);

    // 5. Master: wet/dry mix, volume, mono bass, clip, bypass
    masterModule.getDSP().process(ctx);

    visualizerSource.pushOutput(buffer.getArrayOfReadPointers(), buffer.getNumChannels(), numSamples);
    visualizerSource.pushGainReductionValue(0.0f, numSamples);
}

void WvZzReverbAudioProcessor::randomizeParameters()
{
    juce::Random rng;

    // { parameter id, lowest normalized value, highest normalized value }
    struct Bounds { const char* id; float lo; float hi; };
    static constexpr Bounds bounds[] = {
        { "rev_predelay", 0.0f, 0.5f }, { "rev_size", 0.1f, 1.0f }, { "rev_decay", 0.2f, 0.8f },
        { "rev_lodamp", 0.1f, 0.6f }, { "rev_hidamp", 0.3f, 0.9f }, { "rev_diffusion", 0.4f, 1.0f },
        { "rev_mod", 0.0f, 0.6f }, { "rev_width", 0.6f, 1.0f }, { "rev_shimmer", 0.2f, 0.8f },
        { "rev_mix", 0.2f, 0.5f }, { "er_size", 0.1f, 1.0f }, { "er_diffusion", 0.2f, 0.9f }, { "er_level", 0.1f, 0.7f }
    };

    for (const auto& b : bounds)
    {
        if (auto* param = apvts.getParameter(b.id))
            param->setValueNotifyingHost(b.lo + rng.nextFloat() * (b.hi - b.lo));
    }
}

juce::AudioProcessorEditor* WvZzReverbAudioProcessor::createEditor()
{
    return new WvZzReverbAudioProcessorEditor(*this);
}

bool WvZzReverbAudioProcessor::hasEditor() const { return true; }

void WvZzReverbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    // Keep the selected factory preset with the state, so the preset box matches after a reload.
    state.setProperty("currentProgram", currentProgramIndex, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WvZzReverbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        const auto state = juce::ValueTree::fromXml(*xmlState);
        apvts.replaceState(state);
        currentProgramIndex = juce::jlimit(0, getNumPrograms() - 1, static_cast<int>(state.getProperty("currentProgram", 0)));
    }
}

} // namespace wvzz

#ifndef WVZZ_NO_CREATE_PLUGIN_FILTER
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new wvzz::WvZzReverbAudioProcessor();
}
#endif
