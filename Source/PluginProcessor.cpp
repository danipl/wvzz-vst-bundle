#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <random>

CamelPhatAudioProcessor::CamelPhatAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", camelphat::createParameterLayout())
{
    paramPointers.initialize(apvts);
}

CamelPhatAudioProcessor::~CamelPhatAudioProcessor()
{
}

const juce::String CamelPhatAudioProcessor::getName() const
{
    return "CamelPhat";
}

bool CamelPhatAudioProcessor::acceptsMidi() const
{
    return true;
}

bool CamelPhatAudioProcessor::producesMidi() const
{
    return false;
}

bool CamelPhatAudioProcessor::isMidiEffect() const
{
    return false;
}

double CamelPhatAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CamelPhatAudioProcessor::getNumPrograms()
{
    return static_cast<int>(camelphat::getFactoryPresets().size());
}

int CamelPhatAudioProcessor::getCurrentProgram()
{
    return currentProgramIndex;
}

void CamelPhatAudioProcessor::setCurrentProgram(int index)
{
    const auto& presets = camelphat::getFactoryPresets();
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

const juce::String CamelPhatAudioProcessor::getProgramName(int index)
{
    const auto& presets = camelphat::getFactoryPresets();
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[static_cast<size_t>(index)].name;
    return {};
}

void CamelPhatAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void CamelPhatAudioProcessor::initSmoothers(double sampleRate) noexcept
{
    constexpr double rampTimeSec = 0.015; // 15ms smoothing ramp

    inputGainSmoothed.reset(sampleRate, rampTimeSec);
    bpLowCutSmoothed.reset(sampleRate, rampTimeSec);
    bpHighCutSmoothed.reset(sampleRate, rampTimeSec);
    bpLowResSmoothed.reset(sampleRate, rampTimeSec);
    bpHighResSmoothed.reset(sampleRate, rampTimeSec);
    bpBrMixSmoothed.reset(sampleRate, rampTimeSec);

    distMechSmoothed.reset(sampleRate, rampTimeSec);
    distTubeSmoothed.reset(sampleRate, rampTimeSec);
    distBitSmoothed.reset(sampleRate, rampTimeSec);
    distXcitaSmoothed.reset(sampleRate, rampTimeSec);

    mmCutoffSmoothed.reset(sampleRate, rampTimeSec);
    mmResSmoothed.reset(sampleRate, rampTimeSec);
    mmMixSmoothed.reset(sampleRate, rampTimeSec);
    mmEnvDepthSmoothed.reset(sampleRate, rampTimeSec);

    flgRateSmoothed.reset(sampleRate, rampTimeSec);
    flgAmountSmoothed.reset(sampleRate, rampTimeSec);

    meqAmountSmoothed.reset(sampleRate, rampTimeSec);
    meqTuneSmoothed.reset(sampleRate, rampTimeSec);

    compAmountSmoothed.reset(sampleRate, rampTimeSec);
    compReleaseSmoothed.reset(sampleRate, rampTimeSec);

    masterMixSmoothed.reset(sampleRate, rampTimeSec);
    masterVolSmoothed.reset(sampleRate, rampTimeSec);

    if (paramPointers.master.inputGain != nullptr)
    {
        inputGainSmoothed.setCurrentAndTargetValue(paramPointers.master.inputGain->load());
        bpLowCutSmoothed.setCurrentAndTargetValue(paramPointers.bp.lowCut->load());
        bpHighCutSmoothed.setCurrentAndTargetValue(paramPointers.bp.highCut->load());
        bpLowResSmoothed.setCurrentAndTargetValue(paramPointers.bp.lowRes->load());
        bpHighResSmoothed.setCurrentAndTargetValue(paramPointers.bp.highRes->load());
        bpBrMixSmoothed.setCurrentAndTargetValue(paramPointers.bp.brMix->load());

        distMechSmoothed.setCurrentAndTargetValue(paramPointers.dist.mech->load());
        distTubeSmoothed.setCurrentAndTargetValue(paramPointers.dist.tube->load());
        distBitSmoothed.setCurrentAndTargetValue(paramPointers.dist.bit->load());
        distXcitaSmoothed.setCurrentAndTargetValue(paramPointers.dist.xcita->load());

        mmCutoffSmoothed.setCurrentAndTargetValue(paramPointers.mm.cutoff->load());
        mmResSmoothed.setCurrentAndTargetValue(paramPointers.mm.res->load());
        mmMixSmoothed.setCurrentAndTargetValue(paramPointers.mm.mix->load());
        mmEnvDepthSmoothed.setCurrentAndTargetValue(paramPointers.mm.envDepth->load());

        flgRateSmoothed.setCurrentAndTargetValue(paramPointers.flanger.rate->load());
        flgAmountSmoothed.setCurrentAndTargetValue(paramPointers.flanger.amount->load());

        meqAmountSmoothed.setCurrentAndTargetValue(paramPointers.magicEq.amount->load());
        meqTuneSmoothed.setCurrentAndTargetValue(paramPointers.magicEq.tune->load());

        compAmountSmoothed.setCurrentAndTargetValue(paramPointers.comp.amount->load());
        compReleaseSmoothed.setCurrentAndTargetValue(paramPointers.comp.release->load());

        masterMixSmoothed.setCurrentAndTargetValue(paramPointers.master.mix->load());
        masterVolSmoothed.setCurrentAndTargetValue(paramPointers.master.vol->load());
    }
}

void CamelPhatAudioProcessor::updateSmootherTargets() noexcept
{
    inputGainSmoothed.setTargetValue(paramPointers.master.inputGain->load(std::memory_order_relaxed));

    const float rawLow = paramPointers.bp.lowCut->load(std::memory_order_relaxed);
    const float rawHigh = paramPointers.bp.highCut->load(std::memory_order_relaxed);
    const float curLow = std::min(rawLow, rawHigh);
    const float curHigh = std::max(rawLow, rawHigh);

    bpLowCutSmoothed.setTargetValue(curLow);
    bpHighCutSmoothed.setTargetValue(curHigh);
    bpLowResSmoothed.setTargetValue(paramPointers.bp.lowRes->load(std::memory_order_relaxed));
    bpHighResSmoothed.setTargetValue(paramPointers.bp.highRes->load(std::memory_order_relaxed));
    bpBrMixSmoothed.setTargetValue(paramPointers.bp.brMix->load(std::memory_order_relaxed));

    distMechSmoothed.setTargetValue(paramPointers.dist.mech->load(std::memory_order_relaxed));
    distTubeSmoothed.setTargetValue(paramPointers.dist.tube->load(std::memory_order_relaxed));
    distBitSmoothed.setTargetValue(paramPointers.dist.bit->load(std::memory_order_relaxed));
    distXcitaSmoothed.setTargetValue(paramPointers.dist.xcita->load(std::memory_order_relaxed));

    mmCutoffSmoothed.setTargetValue(paramPointers.mm.cutoff->load(std::memory_order_relaxed));
    mmResSmoothed.setTargetValue(paramPointers.mm.res->load(std::memory_order_relaxed));
    mmMixSmoothed.setTargetValue(paramPointers.mm.mix->load(std::memory_order_relaxed));
    mmEnvDepthSmoothed.setTargetValue(paramPointers.mm.envDepth->load(std::memory_order_relaxed));

    flgRateSmoothed.setTargetValue(paramPointers.flanger.rate->load(std::memory_order_relaxed));
    flgAmountSmoothed.setTargetValue(paramPointers.flanger.amount->load(std::memory_order_relaxed));

    meqAmountSmoothed.setTargetValue(paramPointers.magicEq.amount->load(std::memory_order_relaxed));
    meqTuneSmoothed.setTargetValue(paramPointers.magicEq.tune->load(std::memory_order_relaxed));

    compAmountSmoothed.setTargetValue(paramPointers.comp.amount->load(std::memory_order_relaxed));
    compReleaseSmoothed.setTargetValue(paramPointers.comp.release->load(std::memory_order_relaxed));

    masterMixSmoothed.setTargetValue(paramPointers.master.mix->load(std::memory_order_relaxed));
    masterVolSmoothed.setTargetValue(paramPointers.master.vol->load(std::memory_order_relaxed));
}

void CamelPhatAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    preBpInputBuffer.setSize(2, samplesPerBlock, false, false, true);
    globalDryBuffer.setSize(2, samplesPerBlock, false, false, true);
    bandRejectBuffer.setSize(2, samplesPerBlock, false, false, true);
    passbandBuffer.setSize(2, samplesPerBlock, false, false, true);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    bpFilter.prepare(spec);
    distortionModule.prepare(spec);
    mmFilter.prepare(spec);
    flangerModule.prepare(spec);
    magicEq.prepare(spec);
    compressorModule.prepare(spec);
    masterSection.prepare(spec);
    modEngine.prepare(sampleRate);
    inputMeterSource.reset();
    visualizerSource.prepare(sampleRate, samplesPerBlock);

    // Set oversampling latency on master section and report to host
    const int latency = distortionModule.getLatencyInSamples();
    masterSection.setLatencyCompensation(latency);
    setLatencySamples(latency);

    paramPointers.initialize(apvts);
    initSmoothers(sampleRate);
}

void CamelPhatAudioProcessor::releaseResources()
{
    bpFilter.reset();
    distortionModule.reset();
    mmFilter.reset();
    flangerModule.reset();
    magicEq.reset();
    compressorModule.reset();
    masterSection.reset();
    modEngine.reset();
    inputMeterSource.reset();
    visualizerSource.reset();
}

bool CamelPhatAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void CamelPhatAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const auto numSamples             = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    if (totalNumInputChannels < 2 || numSamples == 0)
        return;

    updateSmootherTargets();

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

        // Advance smoothers for sub-block
        const float curInGain  = inputGainSmoothed.skip(subBlockSize);
        const float curLowCut  = bpLowCutSmoothed.skip(subBlockSize);
        const float curHighCut = bpHighCutSmoothed.skip(subBlockSize);
        const float curLowRes  = bpLowResSmoothed.skip(subBlockSize);
        const float curHighRes = bpHighResSmoothed.skip(subBlockSize);
        const float curBrMix   = bpBrMixSmoothed.skip(subBlockSize);

        const float curMech  = distMechSmoothed.skip(subBlockSize);
        const float curTube  = distTubeSmoothed.skip(subBlockSize);
        const float curBit   = distBitSmoothed.skip(subBlockSize);
        const float curXcita = distXcitaSmoothed.skip(subBlockSize);

        const float curMmCut  = mmCutoffSmoothed.skip(subBlockSize);
        const float curMmRes  = mmResSmoothed.skip(subBlockSize);
        const float curMmMix  = mmMixSmoothed.skip(subBlockSize);
        const float curMmDepth = mmEnvDepthSmoothed.skip(subBlockSize);

        const float curFlRate = flgRateSmoothed.skip(subBlockSize);
        const float curFlAmt  = flgAmountSmoothed.skip(subBlockSize);

        const float curEqAmt  = meqAmountSmoothed.skip(subBlockSize);
        const float curEqTune = meqTuneSmoothed.skip(subBlockSize);

        const float curCompAmt = compAmountSmoothed.skip(subBlockSize);
        const float curCompRel = compReleaseSmoothed.skip(subBlockSize);

        const float curMasterMix = masterMixSmoothed.skip(subBlockSize);
        const float curMasterVol = masterVolSmoothed.skip(subBlockSize);

        // Discrete and bypass parameter loads
        const bool bpByp   = paramPointers.bp.bypass->load(std::memory_order_relaxed) > 0.5f;
        const bool distByp = paramPointers.dist.bypass->load(std::memory_order_relaxed) > 0.5f;
        const bool mmByp   = paramPointers.mm.bypass->load(std::memory_order_relaxed) > 0.5f;
        const int  mmT     = static_cast<int>(paramPointers.mm.type->load(std::memory_order_relaxed));
        const float mmAtt  = paramPointers.mm.envAtt->load(std::memory_order_relaxed);
        const float mmRel  = paramPointers.mm.envRel->load(std::memory_order_relaxed);

        const bool flgByp  = paramPointers.flanger.bypass->load(std::memory_order_relaxed) > 0.5f;
        const bool meqByp  = paramPointers.magicEq.bypass->load(std::memory_order_relaxed) > 0.5f;
        const bool meqPhat = paramPointers.magicEq.phat->load(std::memory_order_relaxed) > 0.5f;

        const bool compByp  = paramPointers.comp.bypass->load(std::memory_order_relaxed) > 0.5f;
        const bool compPhat = paramPointers.comp.phat->load(std::memory_order_relaxed) > 0.5f;

        const bool masterByp = paramPointers.master.bypass->load(std::memory_order_relaxed) > 0.5f;

        // 1. Update Modulation Engine for this 32-sample sub-block
        modEngine.updateBlockControl(paramPointers.mod, bpm, subBlockSize);

        // Fetch modulated targets
        const float modLowCut  = juce::jlimit(20.0f, 20000.0f, curLowCut * std::pow(2.0f, (modEngine.getOffset(camelphat::ModTarget::BPLowCut) + modEngine.getOffset(camelphat::ModTarget::BPBothCuts)) * 4.0f));
        const float modHighCut = juce::jlimit(modLowCut, 20000.0f, curHighCut * std::pow(2.0f, (modEngine.getOffset(camelphat::ModTarget::BPHighCut) + modEngine.getOffset(camelphat::ModTarget::BPBothCuts)) * 4.0f));
        const float modLowRes  = juce::jlimit(0.0f, 1.0f, curLowRes + modEngine.getOffset(camelphat::ModTarget::BPLowRes));
        const float modHighRes = juce::jlimit(0.0f, 1.0f, curHighRes + modEngine.getOffset(camelphat::ModTarget::BPHighRes));
        const float modBrMix   = juce::jlimit(0.0f, 1.0f, curBrMix + modEngine.getOffset(camelphat::ModTarget::BPBrMix));

        const float modMech  = juce::jlimit(0.0f, 1.0f, curMech + modEngine.getOffset(camelphat::ModTarget::DistMech));
        const float modTube  = juce::jlimit(0.0f, 1.0f, curTube + modEngine.getOffset(camelphat::ModTarget::DistTube));
        const float modBit   = juce::jlimit(0.0f, 1.0f, curBit + modEngine.getOffset(camelphat::ModTarget::DistBit));
        const float modXcita = juce::jlimit(0.0f, 1.0f, curXcita + modEngine.getOffset(camelphat::ModTarget::DistXcita));

        const float modMmCut = juce::jlimit(20.0f, 20000.0f, curMmCut * std::pow(2.0f, modEngine.getOffset(camelphat::ModTarget::MMCutoff) * 4.0f));
        const float modMmRes = juce::jlimit(0.0f, 1.0f, curMmRes + modEngine.getOffset(camelphat::ModTarget::MMRes));
        const float modMmMix = juce::jlimit(0.0f, 1.0f, curMmMix + modEngine.getOffset(camelphat::ModTarget::MMMix));

        const float modFlgRate = juce::jlimit(0.01f, 20.0f, curFlRate * std::pow(2.0f, modEngine.getOffset(camelphat::ModTarget::FlangerRate) * 3.0f));
        const float modFlgAmt  = juce::jlimit(0.0f, 1.0f, curFlAmt + modEngine.getOffset(camelphat::ModTarget::FlangerAmount));

        const float modEqAmt  = juce::jlimit(0.0f, 1.0f, curEqAmt + modEngine.getOffset(camelphat::ModTarget::MagicEQAmount));
        const float modEqTune = juce::jlimit(60.0f, 120.0f, curEqTune + modEngine.getOffset(camelphat::ModTarget::MagicEQTune) * 30.0f);

        const float modCompAmt = juce::jlimit(0.0f, 1.0f, curCompAmt + modEngine.getOffset(camelphat::ModTarget::CompAmount));
        const float modMasterMix = juce::jlimit(0.0f, 1.0f, curMasterMix + modEngine.getOffset(camelphat::ModTarget::MasterMix));

        // Create audio views for this sub-block
        juce::AudioBuffer<float> subIn(buffer.getArrayOfWritePointers(), 2, samplesProcessed, subBlockSize);
        juce::AudioBuffer<float> subPreBp(preBpInputBuffer.getArrayOfWritePointers(), 2, 0, subBlockSize);
        juce::AudioBuffer<float> subDry(globalDryBuffer.getArrayOfWritePointers(), 2, 0, subBlockSize);
        juce::AudioBuffer<float> subPass(passbandBuffer.getArrayOfWritePointers(), 2, 0, subBlockSize);
        juce::AudioBuffer<float> subRej(bandRejectBuffer.getArrayOfWritePointers(), 2, 0, subBlockSize);

        // Tap raw incoming signal before input gain is applied
        visualizerSource.pushInput(subIn.getArrayOfReadPointers(), 2, subBlockSize);

        // Apply Input Gain and cache pre-BP buffer
        const float inGainLinear = juce::Decibels::decibelsToGain(curInGain);
        for (int ch = 0; ch < 2; ++ch)
        {
            subIn.applyGain(ch, 0, subBlockSize, inGainLinear);
            subPreBp.copyFrom(ch, 0, subIn.getReadPointer(ch), subBlockSize);
            subDry.copyFrom(ch, 0, subIn.getReadPointer(ch), subBlockSize);
        }

        inputMeterSource.update(subIn, subBlockSize);

        // 1. Module 1: BP Filter (Bandpass + Band-Reject Split)
        bpFilter.process(subIn, subPass, subRej, modLowCut, modHighCut, modLowRes, modHighRes, bpByp);

        // 2. Module 2: Distortion (4 Parallel Circuits + 4x Oversampling)
        distortionModule.setParameters(modMech, modTube, modBit, modXcita, distByp);
        distortionModule.process(subPass);

        // 3. Module 3: MM Filter (Multi-Mode + Envelope Follower tapped from preBpInputBuffer)
        mmFilter.setParameters(modMmCut, modMmRes, mmT, modMmMix, mmAtt, mmRel, curMmDepth, mmByp);
        mmFilter.processWithSidechain(subPass, subPreBp);

        // Summing Node: Passband + (Rejected Band * BR Mix) IMMEDIATELY after MM Filter
        if (modBrMix > 0.001f && !bpByp)
        {
            for (int ch = 0; ch < 2; ++ch)
                subPass.addFrom(ch, 0, subRej.getReadPointer(ch), subBlockSize, modBrMix);
        }

        // 4. Module 4: Flanger
        flangerModule.setParameters(modFlgRate, modFlgAmt, flgByp);
        flangerModule.process(subPass);

        // 5. Module 5: Magic EQ
        magicEq.setParameters(modEqAmt, modEqTune, meqPhat, meqByp);
        magicEq.process(subPass);

        // 6. Module 6: Compressor
        compressorModule.setParameters(modCompAmt, curCompRel, compPhat, compByp);
        compressorModule.process(subPass);

        // 7. Module 7: Master Section (Latency compensation + dry/wet blend + volume + bypass)
        masterSection.process(subPass, subDry, modMasterMix, curMasterVol, masterByp);

        // Tap final processed audio after master section and output volume
        visualizerSource.pushOutput(subPass.getArrayOfReadPointers(), 2, subBlockSize);

        // Copy processed sub-block back to main buffer
        for (int ch = 0; ch < 2; ++ch)
            buffer.copyFrom(ch, samplesProcessed, subPass.getReadPointer(ch), subBlockSize);

        samplesProcessed += subBlockSize;
    }
}

void CamelPhatAudioProcessor::randomizeParameters()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    // Randomize modules within musical bounds:
    // Keep master_vol at 0 dB, master_mix in [0.4, 1.0]
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::masterVol))
        p->setValueNotifyingHost(p->convertTo0to1(0.0f));
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::masterMix))
        p->setValueNotifyingHost(0.4f + dis(gen) * 0.6f);

    // Prevent extreme simultaneous resonance: at most one filter exceeds 0.75 res
    const float bpRes = dis(gen) * 0.7f;
    const float mmRes = (bpRes > 0.5f) ? (dis(gen) * 0.5f) : (dis(gen) * 0.85f);

    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::bpLowRes))  p->setValueNotifyingHost(bpRes);
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::bpHighRes)) p->setValueNotifyingHost(bpRes);
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::mmRes))     p->setValueNotifyingHost(mmRes);

    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::distMech))  p->setValueNotifyingHost(dis(gen) * 0.8f);
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::distTube))  p->setValueNotifyingHost(dis(gen) * 0.8f);
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::distBit))   p->setValueNotifyingHost(dis(gen) * 0.5f);
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::distXcita)) p->setValueNotifyingHost(dis(gen) * 0.6f);

    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::mmType))
        p->setValueNotifyingHost(static_cast<float>(dis(gen) * 11.0f) / 11.0f);

    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::meqAmount))  p->setValueNotifyingHost(dis(gen) * 0.7f);
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::compAmount)) p->setValueNotifyingHost(dis(gen) * 0.75f);
}

bool CamelPhatAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* CamelPhatAudioProcessor::createEditor()
{
    return new CamelPhatAudioProcessorEditor(*this);
}

void CamelPhatAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CamelPhatAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CamelPhatAudioProcessor();
}
