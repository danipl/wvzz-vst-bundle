#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Core/UI/LfoPhaseScopeComponent.h"

namespace wvzz {

WvZzModulationAudioProcessor::WvZzModulationAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      chorusModule("chor"),
      phaserModule("phs"),
      flangerModule("flg", true),
      tremoloModule("trem"),
      modModule("mod", &visualizerSource, ModProfile::Modulation),
      masterModule("master"),
      apvts(*this, nullptr, "Parameters",
            createAggregatedLayout(chorusModule, phaserModule, flangerModule, tremoloModule, modModule, masterModule))
{
    chorusModule.getDSP().bindParameters(apvts);
    phaserModule.getDSP().bindParameters(apvts);
    flangerModule.getDSP().bindParameters(apvts);
    tremoloModule.getDSP().bindParameters(apvts);
    modModule.getDSP().bindParameters(apvts);
    masterModule.getDSP().bindParameters(apvts);

    // Custom center tab: LFO phase scope. It reads only the lock-free LFO taps of the four effects.
    modModule.setCustomDisplay("SCOPE", [this]() -> std::unique_ptr<juce::Component> {
        return std::make_unique<ui::LfoPhaseScopeComponent>(std::vector<ui::LfoPhaseScopeComponent::Lane> {
            { "CHORUS",  ui::WvZzLookAndFeel::skyCyan,        &chorusModule.getChorusDSP().getLfoTap() },
            { "PHASER",  ui::WvZzLookAndFeel::emeraldGreen,   &phaserModule.getPhaserDSP().getLfoTap() },
            { "FLANGER", ui::WvZzLookAndFeel::electricViolet, &flangerModule.getFlangerDSP().getLfoTap() },
            { "TREMOLO", ui::WvZzLookAndFeel::warmAmber,      &tremoloModule.getTremoloDSP().getLfoTap() }
        });
    });

    // Start on the first factory preset, so the plugin does not open with every effect bypassed.
    setCurrentProgram(0);
}

WvZzModulationAudioProcessor::~WvZzModulationAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout WvZzModulationAudioProcessor::createAggregatedLayout(
    modules::ChorusModule& chorus,
    modules::PhaserModule& phaser,
    modules::FlangerModule& flanger,
    modules::TremoloModule& tremolo,
    modules::ModulationModule& mod,
    modules::MasterModule& master)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    chorus.getDSP().registerParameters(layout);
    phaser.getDSP().registerParameters(layout);
    flanger.getDSP().registerParameters(layout);
    tremolo.getDSP().registerParameters(layout);
    mod.getDSP().registerParameters(layout);
    master.getDSP().registerParameters(layout);
    return layout;
}

const juce::String WvZzModulationAudioProcessor::getName() const
{
    return "WvZz Modulation";
}

bool WvZzModulationAudioProcessor::acceptsMidi() const { return true; }
bool WvZzModulationAudioProcessor::producesMidi() const { return false; }
bool WvZzModulationAudioProcessor::isMidiEffect() const { return false; }
double WvZzModulationAudioProcessor::getTailLengthSeconds() const { return tailLengthSeconds; }

int WvZzModulationAudioProcessor::getNumPrograms() { return static_cast<int>(modulationfx::getFactoryPresets().size()); }
int WvZzModulationAudioProcessor::getCurrentProgram() { return currentProgramIndex; }

void WvZzModulationAudioProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return;

    currentProgramIndex = index;
    const auto& preset = modulationfx::getFactoryPresets()[static_cast<size_t>(index)];

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

const juce::String WvZzModulationAudioProcessor::getProgramName(int index)
{
    const auto& presets = modulationfx::getFactoryPresets();
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[static_cast<size_t>(index)].name;
    return {};
}

void WvZzModulationAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void WvZzModulationAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec {
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        2
    };

    visualizerSource.prepare(sampleRate, samplesPerBlock);

    chorusModule.getDSP().prepare(spec);
    phaserModule.getDSP().prepare(spec);
    flangerModule.getDSP().prepare(spec);
    tremoloModule.getDSP().prepare(spec);
    modModule.getDSP().prepare(spec);
    masterModule.getDSP().prepare(spec);

    dryInputBuffer.setSize(2, samplesPerBlock, false, false, true);
    auxBuffer.setSize(2, samplesPerBlock, false, false, true);
}

void WvZzModulationAudioProcessor::releaseResources()
{
    chorusModule.getDSP().reset();
    phaserModule.getDSP().reset();
    flangerModule.getDSP().reset();
    tremoloModule.getDSP().reset();
    modModule.getDSP().reset();
    masterModule.getDSP().reset();
    visualizerSource.reset();
}

bool WvZzModulationAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void WvZzModulationAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) noexcept
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

    // 4. Fixed serial chain
    chorusModule.getDSP().process(ctx);
    phaserModule.getDSP().process(ctx);
    flangerModule.getDSP().process(ctx);
    tremoloModule.getDSP().process(ctx);

    // 5. Master: wet/dry mix, volume, mono bass, clip, bypass
    masterModule.getDSP().process(ctx);

    visualizerSource.pushOutput(buffer.getArrayOfReadPointers(), buffer.getNumChannels(), numSamples);
    visualizerSource.pushGainReductionValue(0.0f, numSamples);
}

void WvZzModulationAudioProcessor::randomizeParameters()
{
    juce::Random rng;

    // { parameter id, lowest normalized value, highest normalized value }
    struct Bounds { const char* id; float lo; float hi; };
    static constexpr Bounds bounds[] = {
        { "chor_rate", 0.1f, 0.7f }, { "chor_depth", 0.15f, 0.7f }, { "chor_delay", 0.1f, 0.8f },
        { "chor_spread", 0.4f, 1.0f }, { "chor_feedback", 0.0f, 0.4f }, { "chor_mix", 0.25f, 0.6f },
        { "phs_rate", 0.1f, 0.7f }, { "phs_depth", 0.3f, 0.9f }, { "phs_center", 0.2f, 0.8f },
        { "phs_feedback", 0.3f, 0.9f }, { "phs_stereo", 0.0f, 1.0f }, { "phs_mix", 0.3f, 0.6f },
        { "flg_rate", 0.1f, 0.6f }, { "flg_amount", 0.2f, 0.55f }, { "flg_feedback", 0.2f, 0.9f },
        { "flg_depth", 0.2f, 0.7f }, { "flg_center", 0.1f, 0.5f },
        { "trem_rate", 0.2f, 0.8f }, { "trem_depth", 0.2f, 0.8f }, { "trem_stereo", 0.0f, 1.0f },
        { "trem_xover", 0.2f, 0.8f }
    };

    for (const auto& b : bounds)
    {
        if (auto* param = apvts.getParameter(b.id))
            param->setValueNotifyingHost(b.lo + rng.nextFloat() * (b.hi - b.lo));
    }
}

juce::AudioProcessorEditor* WvZzModulationAudioProcessor::createEditor()
{
    return new WvZzModulationAudioProcessorEditor(*this);
}

bool WvZzModulationAudioProcessor::hasEditor() const { return true; }

void WvZzModulationAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    // Keep the selected factory preset with the state, so the preset box matches after a reload.
    state.setProperty("currentProgram", currentProgramIndex, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WvZzModulationAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        const auto state = juce::ValueTree::fromXml(*xmlState);
        apvts.replaceState(state);
        // Sessions saved before this property existed fall back to preset 0.
        currentProgramIndex = juce::jlimit(0, getNumPrograms() - 1, static_cast<int>(state.getProperty("currentProgram", 0)));
    }
}

} // namespace wvzz

#ifndef WVZZ_NO_CREATE_PLUGIN_FILTER
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new wvzz::WvZzModulationAudioProcessor();
}
#endif
