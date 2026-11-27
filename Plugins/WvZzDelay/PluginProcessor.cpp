#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Core/UI/DelayTapDisplayComponent.h"

namespace wvzz {

WvZzDelayAudioProcessor::WvZzDelayAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      delayModule("dly"),
      grainModule("grain"),
      modModule("mod", &visualizerSource, ModProfile::Delay),
      masterModule("master"),
      apvts(*this, nullptr, "Parameters",
            createAggregatedLayout(delayModule, grainModule, modModule, masterModule))
{
    delayModule.getDSP().bindParameters(apvts);
    grainModule.getDSP().bindParameters(apvts);
    modModule.getDSP().bindParameters(apvts);
    masterModule.getDSP().bindParameters(apvts);

    // Granular mode: the Delay module owns the delay lines and asks the Grain module for the grains.
    grainModule.getGrainDSP().setExternalHost(true);
    delayModule.getDelayDSP().setGrainSource(&grainModule.getGrainDSP());

    // Custom center tab: delay-tap display. It reads only the lock-free DelayTapState.
    modModule.setCustomDisplay("TAPS", [this]() -> std::unique_ptr<juce::Component> {
        return std::make_unique<ui::DelayTapDisplayComponent>(delayModule.getDelayDSP().getTapState());
    });

    // Start on the first factory preset.
    setCurrentProgram(0);
}

WvZzDelayAudioProcessor::~WvZzDelayAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout WvZzDelayAudioProcessor::createAggregatedLayout(
    modules::DelayModule& delay,
    modules::GrainModule& grain,
    modules::ModulationModule& mod,
    modules::MasterModule& master)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    delay.getDSP().registerParameters(layout);
    grain.getDSP().registerParameters(layout);
    mod.getDSP().registerParameters(layout);
    master.getDSP().registerParameters(layout);
    return layout;
}

const juce::String WvZzDelayAudioProcessor::getName() const
{
    return "WvZz Delay";
}

bool WvZzDelayAudioProcessor::acceptsMidi() const { return true; }
bool WvZzDelayAudioProcessor::producesMidi() const { return false; }
bool WvZzDelayAudioProcessor::isMidiEffect() const { return false; }

double WvZzDelayAudioProcessor::getTailLengthSeconds() const
{
    // Time until the repeats are 60 dB down for the current time and feedback (1 delay time .. 60 s)
    return delayModule.getDelayDSP().getTailSeconds(lastHostBpm.load(std::memory_order_relaxed));
}


int WvZzDelayAudioProcessor::getNumPrograms() { return static_cast<int>(delayfx::getFactoryPresets().size()); }
int WvZzDelayAudioProcessor::getCurrentProgram() { return currentProgramIndex; }

void WvZzDelayAudioProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return;

    currentProgramIndex = index;
    const auto& preset = delayfx::getFactoryPresets()[static_cast<size_t>(index)];

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

const juce::String WvZzDelayAudioProcessor::getProgramName(int index)
{
    const auto& presets = delayfx::getFactoryPresets();
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[static_cast<size_t>(index)].name;
    return {};
}

void WvZzDelayAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void WvZzDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec {
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        2
    };

    visualizerSource.prepare(sampleRate, samplesPerBlock);

    grainModule.getDSP().prepare(spec);
    delayModule.getDSP().prepare(spec);
    modModule.getDSP().prepare(spec);
    masterModule.getDSP().prepare(spec);

    dryInputBuffer.setSize(2, samplesPerBlock, false, false, true);
    auxBuffer.setSize(2, samplesPerBlock, false, false, true);
}

void WvZzDelayAudioProcessor::releaseResources()
{
    delayModule.getDSP().reset();
    grainModule.getDSP().reset();
    modModule.getDSP().reset();
    masterModule.getDSP().reset();
    visualizerSource.reset();
}

bool WvZzDelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void WvZzDelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) noexcept
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

    lastHostBpm.store(ctx.hostBpm, std::memory_order_relaxed);

    // 4. Delay. In Granular mode it calls the Grain module for the repeats.
    delayModule.getDSP().process(ctx);

    // 5. Master: wet/dry mix, volume, mono bass, clip, bypass
    masterModule.getDSP().process(ctx);

    visualizerSource.pushOutput(buffer.getArrayOfReadPointers(), buffer.getNumChannels(), numSamples);
    visualizerSource.pushGainReductionValue(0.0f, numSamples);
}

void WvZzDelayAudioProcessor::randomizeParameters()
{
    juce::Random rng;

    // { parameter id, lowest normalized value, highest normalized value }
    struct Bounds { const char* id; float lo; float hi; };
    static constexpr Bounds bounds[] = {
        { "dly_time", 0.15f, 0.7f }, { "dly_feedback", 0.15f, 0.7f }, { "dly_hp", 0.1f, 0.6f },
        { "dly_lp", 0.4f, 0.9f }, { "dly_diffusion", 0.0f, 0.7f }, { "dly_duck", 0.0f, 0.6f },
        { "dly_wow", 0.1f, 0.7f }, { "dly_drive", 0.1f, 0.7f }, { "dly_mix", 0.2f, 0.5f },
        { "grain_size", 0.2f, 0.7f }, { "grain_density", 0.3f, 0.8f }, { "grain_spray", 0.0f, 0.6f },
        { "grain_reverse", 0.0f, 0.4f }, { "grain_spread", 0.3f, 1.0f }
    };

    for (const auto& b : bounds)
    {
        if (auto* param = apvts.getParameter(b.id))
            param->setValueNotifyingHost(b.lo + rng.nextFloat() * (b.hi - b.lo));
    }
}

juce::AudioProcessorEditor* WvZzDelayAudioProcessor::createEditor()
{
    return new WvZzDelayAudioProcessorEditor(*this);
}

bool WvZzDelayAudioProcessor::hasEditor() const { return true; }

void WvZzDelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    // Keep the selected factory preset with the state, so the preset box matches after a reload.
    state.setProperty("currentProgram", currentProgramIndex, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WvZzDelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
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
    return new wvzz::WvZzDelayAudioProcessor();
}
#endif
