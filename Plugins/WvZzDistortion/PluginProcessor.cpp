#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <random>

namespace wvzz {

juce::AudioProcessorValueTreeState::ParameterLayout WvZzDistortionAudioProcessor::createAggregatedLayout(
    modules::BPFilterModule& bp,
    modules::DistortionModule& dist,
    modules::MMFilterModule& mm,
    modules::FlangerModule& flg,
    modules::MagicEQModule& meq,
    modules::CompressorModule& comp,
    modules::ModulationModule& mod,
    modules::MasterModule& master)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    bp.getDSP().registerParameters(layout);
    dist.getDSP().registerParameters(layout);
    mm.getDSP().registerParameters(layout);
    flg.getDSP().registerParameters(layout);
    meq.getDSP().registerParameters(layout);
    comp.getDSP().registerParameters(layout);
    mod.getDSP().registerParameters(layout);
    master.getDSP().registerParameters(layout);

    return layout;
}

WvZzDistortionAudioProcessor::WvZzDistortionAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      bpModule("bp"),
      distModule("dist"),
      mmModule("mm"),
      flgModule("flg"),
      meqModule("meq"),
      compModule("comp"),
      modModule("mod"),
      masterModule("master"),
      apvts(*this, nullptr, "Parameters", createAggregatedLayout(bpModule, distModule, mmModule, flgModule, meqModule, compModule, modModule, masterModule))
{
    // Bind all modules to APVTS parameters
    bpModule.getDSP().bindParameters(apvts);
    distModule.getDSP().bindParameters(apvts);
    mmModule.getDSP().bindParameters(apvts);
    flgModule.getDSP().bindParameters(apvts);
    meqModule.getDSP().bindParameters(apvts);
    compModule.getDSP().bindParameters(apvts);
    modModule.getDSP().bindParameters(apvts);
    masterModule.getDSP().bindParameters(apvts);
}

WvZzDistortionAudioProcessor::~WvZzDistortionAudioProcessor()
{
}

const juce::String WvZzDistortionAudioProcessor::getName() const
{
    return "WvZz Distortion";
}

bool WvZzDistortionAudioProcessor::acceptsMidi() const
{
    return true;
}

bool WvZzDistortionAudioProcessor::producesMidi() const
{
    return false;
}

bool WvZzDistortionAudioProcessor::isMidiEffect() const
{
    return false;
}

double WvZzDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WvZzDistortionAudioProcessor::getNumPrograms()
{
    return static_cast<int>(getFactoryPresets().size());
}

int WvZzDistortionAudioProcessor::getCurrentProgram()
{
    return currentProgramIndex;
}

void WvZzDistortionAudioProcessor::setCurrentProgram(int index)
{
    const auto& presets = getFactoryPresets();
    if (index >= 0 && index < static_cast<int>(presets.size()))
    {
        currentProgramIndex = index;
        const auto& p = presets[static_cast<size_t>(index)];
        for (const auto& [paramId, val] : p.values)
        {
            if (auto* param = apvts.getParameter(paramId))
            {
                param->setValueNotifyingHost(param->convertTo0to1(val));
            }
        }
    }
}

const juce::String WvZzDistortionAudioProcessor::getProgramName(int index)
{
    const auto& presets = getFactoryPresets();
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[static_cast<size_t>(index)].name;
    return {};
}

void WvZzDistortionAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void WvZzDistortionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    preBpInputBuffer.setSize(2, samplesPerBlock, false, false, true);
    bandRejectBuffer.setSize(2, samplesPerBlock, false, false, true);
    passbandBuffer.setSize(2, samplesPerBlock, false, false, true);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    bpModule.getDSP().prepare(spec);
    distModule.getDSP().prepare(spec);
    mmModule.getDSP().prepare(spec);
    flgModule.getDSP().prepare(spec);
    meqModule.getDSP().prepare(spec);
    compModule.getDSP().prepare(spec);
    modModule.getDSP().prepare(spec);
    masterModule.getDSP().prepare(spec);

    visualizerSource.prepare(sampleRate, samplesPerBlock);

    // Latency compensation for distortion oversampling
    const int latency = distModule.getDistortionDSP().getLatencyInSamples();
    masterModule.getMasterDSP().setLatencyCompensation(latency);
    setLatencySamples(latency);
}

void WvZzDistortionAudioProcessor::releaseResources()
{
    bpModule.getDSP().reset();
    distModule.getDSP().reset();
    mmModule.getDSP().reset();
    flgModule.getDSP().reset();
    meqModule.getDSP().reset();
    compModule.getDSP().reset();
    modModule.getDSP().reset();
    masterModule.getDSP().reset();
    visualizerSource.reset();
}

bool WvZzDistortionAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void WvZzDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) noexcept
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const auto numSamples             = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    if (totalNumInputChannels < 2 || numSamples == 0)
        return;

    // Host Tempo Query
    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
    {
        if (auto pos = playHead->getPosition())
        {
            if (pos->getBpm().hasValue())
                bpm = *pos->getBpm();
        }
    }

    // Process in sub-blocks of up to 32 samples for control-rate modulation
    int samplesProcessed = 0;
    while (samplesProcessed < numSamples)
    {
        const int subBlockSize = std::min(32, numSamples - samplesProcessed);

        // Audio views for this sub-block
        juce::AudioBuffer<float> subIn(buffer.getArrayOfWritePointers(), 2, samplesProcessed, subBlockSize);
        juce::AudioBuffer<float> subPreBp(preBpInputBuffer.getArrayOfWritePointers(), 2, 0, subBlockSize);
        juce::AudioBuffer<float> subPass(passbandBuffer.getArrayOfWritePointers(), 2, 0, subBlockSize);
        juce::AudioBuffer<float> subRej(bandRejectBuffer.getArrayOfWritePointers(), 2, 0, subBlockSize);

        // Tap raw incoming signal before input gain is applied
        visualizerSource.pushInput(subIn.getArrayOfReadPointers(), 2, subBlockSize);

        // Apply input gain and cache pre-BP sidechain buffer
        masterModule.getMasterDSP().applyInputGain(subIn);
        for (int ch = 0; ch < 2; ++ch)
        {
            subPreBp.copyFrom(ch, 0, subIn.getReadPointer(ch), subBlockSize);
            subPass.copyFrom(ch, 0, subIn.getReadPointer(ch), subBlockSize);
        }

        bool hasAuxSplit = false;

        // Construct ProcessContext for this sub-block
        ProcessContext ctx {
            subPass,
            subPreBp,
            subRej,
            hasAuxSplit,
            nullptr,
            0
        };

        // 1. Modulation Engine computes offsets
        modModule.getDSP().process(ctx);

        // 2. Band-pass Filter (Splits rejected energy into subRej and flags hasAuxSplit)
        bpModule.getDSP().process(ctx);

        // 3. Distortion Module (4-stage saturator)
        distModule.getDSP().process(ctx);

        // 4. Multi-Mode Filter (Uses subPreBp envelope follower)
        mmModule.getDSP().process(ctx);

        // 5. Band-pass Recombination (Passband + Rejected Band * BR Mix)
        bpModule.getBPFilterDSP().recombine(ctx);

        // 6. Flanger Module
        flgModule.getDSP().process(ctx);

        // 7. Magic EQ
        meqModule.getDSP().process(ctx);

        // 8. Compressor
        compModule.getDSP().process(ctx);

        // 9. Master Section (Latency compensation, dry delay line, mix, output volume, safety clamp, metering)
        masterModule.getDSP().process(ctx);

        // Tap final processed audio after master section
        visualizerSource.pushOutput(subPass.getArrayOfReadPointers(), 2, subBlockSize);

        // Copy processed sub-block back to host buffer
        for (int ch = 0; ch < 2; ++ch)
            buffer.copyFrom(ch, samplesProcessed, subPass.getReadPointer(ch), subBlockSize);

        samplesProcessed += subBlockSize;
    }
}

void WvZzDistortionAudioProcessor::randomizeParameters()
{
    juce::Random rng;
    bpModule.getDSP().randomize(rng);
    distModule.getDSP().randomize(rng);
    mmModule.getDSP().randomize(rng);
    flgModule.getDSP().randomize(rng);
    meqModule.getDSP().randomize(rng);
    compModule.getDSP().randomize(rng);
    modModule.getDSP().randomize(rng);
    masterModule.getDSP().randomize(rng);
}

bool WvZzDistortionAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* WvZzDistortionAudioProcessor::createEditor()
{
    return new WvZzDistortionAudioProcessorEditor(*this);
}

void WvZzDistortionAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WvZzDistortionAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

} // namespace wvzz

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new wvzz::WvZzDistortionAudioProcessor();
}
