#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace wvzz {

WvZzDynamicsAudioProcessor::WvZzDynamicsAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
                     .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), false)),
      detModule("det"),
      transModule("trans"),
      ottModule("ott"),
      compModule("comp"),
      pumpModule("pump"),
      clipModule("clip"),
      modModule("mod", &visualizerSource, ModProfile::Dynamics),
      masterModule("master"),
      apvts(*this, nullptr, "Parameters",
            createAggregatedLayout(detModule, transModule, ottModule, compModule,
                                   pumpModule, clipModule, modModule, masterModule))
{
    detModule.getDSP().bindParameters(apvts);
    transModule.getDSP().bindParameters(apvts);
    ottModule.getDSP().bindParameters(apvts);
    compModule.getDSP().bindParameters(apvts);
    compModule.getCompressorDSP().setUseSidechainDetector(true);
    pumpModule.getDSP().bindParameters(apvts);
    clipModule.getDSP().bindParameters(apvts);
    modModule.getDSP().bindParameters(apvts);
    masterModule.getDSP().bindParameters(apvts);
}

WvZzDynamicsAudioProcessor::~WvZzDynamicsAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout WvZzDynamicsAudioProcessor::createAggregatedLayout(
    modules::DynDetectorModule& det,
    modules::TransientShaperModule& trans,
    modules::MultibandOTTModule& ott,
    modules::CompressorModule& comp,
    modules::RhythmicPumperModule& pump,
    modules::MaximizerClipperModule& clip,
    modules::ModulationModule& mod,
    modules::MasterModule& master)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    det.getDSP().registerParameters(layout);
    trans.getDSP().registerParameters(layout);
    ott.getDSP().registerParameters(layout);
    comp.getDSP().registerParameters(layout);
    pump.getDSP().registerParameters(layout);
    clip.getDSP().registerParameters(layout);
    mod.getDSP().registerParameters(layout);
    master.getDSP().registerParameters(layout);
    return layout;
}

const juce::String WvZzDynamicsAudioProcessor::getName() const
{
    return "WvZz Dynamics";
}

bool WvZzDynamicsAudioProcessor::acceptsMidi() const { return true; }
bool WvZzDynamicsAudioProcessor::producesMidi() const { return false; }
bool WvZzDynamicsAudioProcessor::isMidiEffect() const { return false; }
double WvZzDynamicsAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int WvZzDynamicsAudioProcessor::getNumPrograms() { return 20; }
int WvZzDynamicsAudioProcessor::getCurrentProgram() { return currentProgramIndex; }

void WvZzDynamicsAudioProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return;

    currentProgramIndex = index;
    const auto& presets = dynamics::getFactoryPresets();
    const auto& p = presets[static_cast<size_t>(index)];

    for (const auto& [paramId, val] : p.values)
    {
        if (auto* param = apvts.getParameter(paramId))
            param->setValueNotifyingHost(param->convertTo0to1(val));
    }
}

const juce::String WvZzDynamicsAudioProcessor::getProgramName(int index)
{
    const auto& presets = dynamics::getFactoryPresets();
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[static_cast<size_t>(index)].name;
    return {};
}

void WvZzDynamicsAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void WvZzDynamicsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec {
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        2
    };

    visualizerSource.prepare(sampleRate, samplesPerBlock);

    detModule.getDSP().prepare(spec);
    transModule.getDSP().prepare(spec);
    ottModule.getDSP().prepare(spec);
    compModule.getDSP().prepare(spec);
    pumpModule.getDSP().prepare(spec);
    clipModule.getDSP().prepare(spec);
    modModule.getDSP().prepare(spec);
    masterModule.getDSP().prepare(spec);

    dryInputBuffer.setSize(2, samplesPerBlock, false, false, true);
    detectorSidechainBuffer.setSize(2, samplesPerBlock, false, false, true);
    subAuxBuffer.setSize(2, samplesPerBlock, false, false, true);
}

void WvZzDynamicsAudioProcessor::releaseResources()
{
    detModule.getDSP().reset();
    transModule.getDSP().reset();
    ottModule.getDSP().reset();
    compModule.getDSP().reset();
    pumpModule.getDSP().reset();
    clipModule.getDSP().reset();
    modModule.getDSP().reset();
    masterModule.getDSP().reset();
    visualizerSource.reset();
}

bool WvZzDynamicsAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Optional Sidechain input bus (stereo or disabled)
    const auto sc = layouts.getChannelSet(true, 1);
    if (!sc.isDisabled() && sc != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void WvZzDynamicsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/) noexcept
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0)
        return;

    // 1. Ensure pre-allocated scratch buffers match block size without heap reallocation
    if (dryInputBuffer.getNumSamples() < numSamples)
        dryInputBuffer.setSize(2, numSamples, false, false, true);
    if (subAuxBuffer.getNumSamples() < numSamples)
        subAuxBuffer.setSize(2, numSamples, false, false, true);
    if (detectorSidechainBuffer.getNumSamples() < numSamples)
        detectorSidechainBuffer.setSize(2, numSamples, false, false, true);

    // 2. Master apply input gain first
    masterModule.getMasterDSP().applyInputGain(buffer);

    // Save clean post-input-gain dry signal for Master wet/dry mix & bypass
    const int numMainChannels = juce::jmin(2, buffer.getNumChannels());
    for (int ch = 0; ch < numMainChannels; ++ch)
        dryInputBuffer.copyFrom(ch, 0, buffer.getReadPointer(ch), numSamples);
    if (numMainChannels == 1)
        dryInputBuffer.copyFrom(1, 0, buffer.getReadPointer(0), numSamples);

    // Populate detectorSidechainBuffer from external sidechain bus if active, else from post-input-gain buffer
    auto scBus = getBusBuffer(buffer, true, 1);
    if (scBus.getNumChannels() >= 2 && scBus.getReadPointer(0) != nullptr)
    {
        for (int ch = 0; ch < 2; ++ch)
            detectorSidechainBuffer.copyFrom(ch, 0, scBus.getReadPointer(ch), numSamples);
    }
    else
    {
        for (int ch = 0; ch < 2; ++ch)
            detectorSidechainBuffer.copyFrom(ch, 0, dryInputBuffer.getReadPointer(ch), numSamples);
    }

    // Capture dry input for visualizer
    visualizerSource.pushInput(buffer.getArrayOfReadPointers(), buffer.getNumChannels(), numSamples);

    // 3. Setup ProcessContext
    bool hasSubSplit = false;
    ProcessContext ctx {
        buffer,
        detectorSidechainBuffer,
        subAuxBuffer,
        hasSubSplit,
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

    // 4. Modulation Module evaluates LFOs, X/Y Pad, and populates offsets
    modModule.getDSP().process(ctx);

    // 5. DynDetector: Sub split (<subXover) & Sidechain Detector filtering
    detModule.getDSP().process(ctx);

    // 6. Transient Shaper: Differential Snap & Gate Floor
    transModule.getDSP().process(ctx);

    // 7. Multiband OTT: 3-Band Upward/Downward compression & High Sheen
    ottModule.getDSP().process(ctx);

    // 8. Bus Compressor: Opto / VCA / FET modes with knee and ADAA
    compModule.getDSP().process(ctx);

    // 9. Recombine isolated Sub-Bass before rhythmic ducking and clipping
    if (hasSubSplit)
    {
        for (int ch = 0; ch < numMainChannels; ++ch)
            buffer.addFrom(ch, 0, subAuxBuffer.getReadPointer(ch), numSamples);
    }

    // 10. Rhythmic Pumper: Host PPQ / Audio sidechain envelope ducking
    pumpModule.getDSP().process(ctx);

    // 11. Maximizer Clipper: Input drive + 1st-order ADAA ceiling
    clipModule.getDSP().process(ctx);

    // If SC Listen is active, audition the filtered sidechain detector signal directly
    if (detModule.getDetectorDSP().isSidechainListen())
    {
        for (int ch = 0; ch < numMainChannels; ++ch)
            buffer.copyFrom(ch, 0, detectorSidechainBuffer.getReadPointer(ch), numSamples);
    }

    // Restore clean dry input into detectorSidechainBuffer so MasterDSP wet/dry mix & bypass blend against true dry signal
    for (int ch = 0; ch < 2; ++ch)
        detectorSidechainBuffer.copyFrom(ch, 0, dryInputBuffer.getReadPointer(ch), numSamples);

    // 12. Master Module: Wet/Dry Mix, Volume, Mono Bass, Clipper
    masterModule.getDSP().process(ctx);

    // Push final output audio and cumulative gain reduction telemetry to visualizer
    visualizerSource.pushOutput(buffer.getArrayOfReadPointers(), buffer.getNumChannels(), numSamples);
    visualizerSource.pushGainReductionValue(ctx.cumulativeGainReductionDb, numSamples);
}

void WvZzDynamicsAudioProcessor::randomizeParameters()
{
    juce::Random rng;
    detModule.getDSP().randomize(rng);
    transModule.getDSP().randomize(rng);
    ottModule.getDSP().randomize(rng);
    compModule.getDSP().randomize(rng);
    pumpModule.getDSP().randomize(rng);
    clipModule.getDSP().randomize(rng);
}

juce::AudioProcessorEditor* WvZzDynamicsAudioProcessor::createEditor()
{
    return new WvZzDynamicsAudioProcessorEditor(*this);
}

bool WvZzDynamicsAudioProcessor::hasEditor() const { return true; }

void WvZzDynamicsAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WvZzDynamicsAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

} // namespace wvzz

#ifndef WVZZ_NO_CREATE_PLUGIN_FILTER
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new wvzz::WvZzDynamicsAudioProcessor();
}
#endif
