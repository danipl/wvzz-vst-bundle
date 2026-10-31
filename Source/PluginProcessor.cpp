#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    return 1;
}

int CamelPhatAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CamelPhatAudioProcessor::setCurrentProgram(int)
{
}

const juce::String CamelPhatAudioProcessor::getProgramName(int)
{
    return {};
}

void CamelPhatAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void CamelPhatAudioProcessor::initSmoothers(double sampleRate) noexcept
{
    constexpr double rampTimeSec = 0.02; // 20ms smoothing ramp to eliminate zipper clicks

    bpCutoffLowSmoothed.reset(sampleRate, rampTimeSec);
    bpCutoffHighSmoothed.reset(sampleRate, rampTimeSec);
    bpResonanceSmoothed.reset(sampleRate, rampTimeSec);
    bpMixSmoothed.reset(sampleRate, rampTimeSec);
    bandRejectMixSmoothed.reset(sampleRate, rampTimeSec);

    distTubeSmoothed.reset(sampleRate, rampTimeSec);
    distMechSmoothed.reset(sampleRate, rampTimeSec);
    distBitSmoothed.reset(sampleRate, rampTimeSec);
    distXcitaSmoothed.reset(sampleRate, rampTimeSec);
    distMixSmoothed.reset(sampleRate, rampTimeSec);

    mmCutoffSmoothed.reset(sampleRate, rampTimeSec);
    mmResonanceSmoothed.reset(sampleRate, rampTimeSec);
    mmEnvAmountSmoothed.reset(sampleRate, rampTimeSec);

    flangerRateSmoothed.reset(sampleRate, rampTimeSec);
    flangerDepthSmoothed.reset(sampleRate, rampTimeSec);
    flangerFeedbackSmoothed.reset(sampleRate, rampTimeSec);
    flangerMixSmoothed.reset(sampleRate, rampTimeSec);

    magicEqAmountSmoothed.reset(sampleRate, rampTimeSec);
    magicEqFreqSmoothed.reset(sampleRate, rampTimeSec);

    compThresholdSmoothed.reset(sampleRate, rampTimeSec);
    compRatioSmoothed.reset(sampleRate, rampTimeSec);
    compMixSmoothed.reset(sampleRate, rampTimeSec);

    masterMixSmoothed.reset(sampleRate, rampTimeSec);
    masterOutputGainSmoothed.reset(sampleRate, rampTimeSec);

    // Seed initial values
    if (paramPointers.bp.cutoffLow != nullptr)
    {
        bpCutoffLowSmoothed.setCurrentAndTargetValue(paramPointers.bp.cutoffLow->load());
        bpCutoffHighSmoothed.setCurrentAndTargetValue(paramPointers.bp.cutoffHigh->load());
        bpResonanceSmoothed.setCurrentAndTargetValue(paramPointers.bp.resonance->load());
        bpMixSmoothed.setCurrentAndTargetValue(paramPointers.bp.mix->load());
        bandRejectMixSmoothed.setCurrentAndTargetValue(paramPointers.bp.bandRejectMix->load());

        distTubeSmoothed.setCurrentAndTargetValue(paramPointers.dist.tube->load());
        distMechSmoothed.setCurrentAndTargetValue(paramPointers.dist.mech->load());
        distBitSmoothed.setCurrentAndTargetValue(paramPointers.dist.bit->load());
        distXcitaSmoothed.setCurrentAndTargetValue(paramPointers.dist.xcita->load());
        distMixSmoothed.setCurrentAndTargetValue(paramPointers.dist.mix->load());

        mmCutoffSmoothed.setCurrentAndTargetValue(paramPointers.mm.cutoff->load());
        mmResonanceSmoothed.setCurrentAndTargetValue(paramPointers.mm.resonance->load());
        mmEnvAmountSmoothed.setCurrentAndTargetValue(paramPointers.mm.envAmount->load());

        flangerRateSmoothed.setCurrentAndTargetValue(paramPointers.flanger.rate->load());
        flangerDepthSmoothed.setCurrentAndTargetValue(paramPointers.flanger.depth->load());
        flangerFeedbackSmoothed.setCurrentAndTargetValue(paramPointers.flanger.feedback->load());
        flangerMixSmoothed.setCurrentAndTargetValue(paramPointers.flanger.mix->load());

        magicEqAmountSmoothed.setCurrentAndTargetValue(paramPointers.magicEq.amount->load());
        magicEqFreqSmoothed.setCurrentAndTargetValue(paramPointers.magicEq.frequency->load());

        compThresholdSmoothed.setCurrentAndTargetValue(paramPointers.comp.threshold->load());
        compRatioSmoothed.setCurrentAndTargetValue(paramPointers.comp.ratio->load());
        compMixSmoothed.setCurrentAndTargetValue(paramPointers.comp.mix->load());

        masterMixSmoothed.setCurrentAndTargetValue(paramPointers.master.mix->load());
        masterOutputGainSmoothed.setCurrentAndTargetValue(paramPointers.master.outputGain->load());
    }
}

void CamelPhatAudioProcessor::updateSmootherTargets() noexcept
{
    bpCutoffLowSmoothed.setTargetValue(paramPointers.bp.cutoffLow->load(std::memory_order_relaxed));
    bpCutoffHighSmoothed.setTargetValue(paramPointers.bp.cutoffHigh->load(std::memory_order_relaxed));
    bpResonanceSmoothed.setTargetValue(paramPointers.bp.resonance->load(std::memory_order_relaxed));
    bpMixSmoothed.setTargetValue(paramPointers.bp.mix->load(std::memory_order_relaxed));
    bandRejectMixSmoothed.setTargetValue(paramPointers.bp.bandRejectMix->load(std::memory_order_relaxed));

    distTubeSmoothed.setTargetValue(paramPointers.dist.tube->load(std::memory_order_relaxed));
    distMechSmoothed.setTargetValue(paramPointers.dist.mech->load(std::memory_order_relaxed));
    distBitSmoothed.setTargetValue(paramPointers.dist.bit->load(std::memory_order_relaxed));
    distXcitaSmoothed.setTargetValue(paramPointers.dist.xcita->load(std::memory_order_relaxed));
    distMixSmoothed.setTargetValue(paramPointers.dist.mix->load(std::memory_order_relaxed));

    mmCutoffSmoothed.setTargetValue(paramPointers.mm.cutoff->load(std::memory_order_relaxed));
    mmResonanceSmoothed.setTargetValue(paramPointers.mm.resonance->load(std::memory_order_relaxed));
    mmEnvAmountSmoothed.setTargetValue(paramPointers.mm.envAmount->load(std::memory_order_relaxed));

    flangerRateSmoothed.setTargetValue(paramPointers.flanger.rate->load(std::memory_order_relaxed));
    flangerDepthSmoothed.setTargetValue(paramPointers.flanger.depth->load(std::memory_order_relaxed));
    flangerFeedbackSmoothed.setTargetValue(paramPointers.flanger.feedback->load(std::memory_order_relaxed));
    flangerMixSmoothed.setTargetValue(paramPointers.flanger.mix->load(std::memory_order_relaxed));

    magicEqAmountSmoothed.setTargetValue(paramPointers.magicEq.amount->load(std::memory_order_relaxed));
    magicEqFreqSmoothed.setTargetValue(paramPointers.magicEq.frequency->load(std::memory_order_relaxed));

    compThresholdSmoothed.setTargetValue(paramPointers.comp.threshold->load(std::memory_order_relaxed));
    compRatioSmoothed.setTargetValue(paramPointers.comp.ratio->load(std::memory_order_relaxed));
    compMixSmoothed.setTargetValue(paramPointers.comp.mix->load(std::memory_order_relaxed));

    masterMixSmoothed.setTargetValue(paramPointers.master.mix->load(std::memory_order_relaxed));
    masterOutputGainSmoothed.setTargetValue(paramPointers.master.outputGain->load(std::memory_order_relaxed));
}

void CamelPhatAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Pre-allocate real-time scratch buffers to ensure zero allocations in processBlock()
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

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    if (totalNumInputChannels < 2 || numSamples == 0)
        return;

    // 0. Cache global dry input buffer for final master crossfade (zero allocation)
    for (int ch = 0; ch < 2; ++ch)
        globalDryBuffer.copyFrom(ch, 0, buffer.getReadPointer(ch), numSamples);

    // Update parameter smoothers
    updateSmootherTargets();

    const float curLowCut  = bpCutoffLowSmoothed.skip(numSamples);
    const float curHighCut = bpCutoffHighSmoothed.skip(numSamples);
    const float curBpRes   = bpResonanceSmoothed.skip(numSamples);
    const float curBpMix   = bpMixSmoothed.skip(numSamples);
    const float curBrMix   = bandRejectMixSmoothed.skip(numSamples);

    const float curTube  = distTubeSmoothed.skip(numSamples);
    const float curMech  = distMechSmoothed.skip(numSamples);
    const float curBit   = distBitSmoothed.skip(numSamples);
    const float curXcita = distXcitaSmoothed.skip(numSamples);
    const float curDistM = distMixSmoothed.skip(numSamples);

    const float curMmCut = mmCutoffSmoothed.skip(numSamples);
    const float curMmRes = mmResonanceSmoothed.skip(numSamples);
    const int   curMmMod = static_cast<int>(paramPointers.mm.mode->load(std::memory_order_relaxed));
    const float curMmEnv = mmEnvAmountSmoothed.skip(numSamples);
    const float curMmAtt = paramPointers.mm.envAttack->load(std::memory_order_relaxed);
    const float curMmRel = paramPointers.mm.envRelease->load(std::memory_order_relaxed);

    const float curFlRate = flangerRateSmoothed.skip(numSamples);
    const float curFlDpth = flangerDepthSmoothed.skip(numSamples);
    const float curFlFdbk = flangerFeedbackSmoothed.skip(numSamples);
    const float curFlMix  = flangerMixSmoothed.skip(numSamples);

    const float curEqAmt  = magicEqAmountSmoothed.skip(numSamples);
    const float curEqFreq = magicEqFreqSmoothed.skip(numSamples);

    const float curCompThresh = compThresholdSmoothed.skip(numSamples);
    const float curCompRatio  = compRatioSmoothed.skip(numSamples);
    const float curCompAtt    = paramPointers.comp.attack->load(std::memory_order_relaxed);
    const float curCompRel    = paramPointers.comp.release->load(std::memory_order_relaxed);
    const bool  curCompPhat   = paramPointers.comp.phatMode->load(std::memory_order_relaxed) > 0.5f;
    const float curCompMix    = compMixSmoothed.skip(numSamples);

    const float curMasterMix  = masterMixSmoothed.skip(numSamples);
    const float curMasterGain = masterOutputGainSmoothed.skip(numSamples);

    // 1. BP Filter Splitter: Separates buffer into passbandBuffer and bandRejectBuffer
    bpFilter.process(buffer, passbandBuffer, bandRejectBuffer, curLowCut, curHighCut, curBpRes, curBpMix);

    // 2. 4-Stage Distortion Module (4x Polyphase IIR Oversampled + 10 Hz DC Blocker)
    distortionModule.setParameters(curTube, curMech, curBit, curXcita, curDistM);
    distortionModule.process(passbandBuffer);

    // 3. Multi-Mode Filter with Envelope Follower
    mmFilter.setParameters(curMmCut, curMmRes, curMmMod, curMmEnv, curMmAtt, curMmRel);
    mmFilter.process(passbandBuffer);

    // 4. Stereo Flanger Module
    flangerModule.setParameters(curFlRate, curFlDpth, curFlFdbk, curFlMix);
    flangerModule.process(passbandBuffer);

    // 5. Band-Reject Summing Node: Recombines wet path with bandRejectBuffer * bandRejectMix
    if (curBrMix > 0.001f)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            passbandBuffer.addFrom(ch, 0, bandRejectBuffer.getReadPointer(ch), numSamples, curBrMix);
        }
    }

    // 6. Magic EQ (Resonant Sub-Bass Boost + Warmth)
    magicEq.setParameters(curEqAmt, curEqFreq);
    magicEq.process(passbandBuffer);

    // 7. Optical Compressor with "Phat" Mode
    compressorModule.setParameters(curCompThresh, curCompRatio, curCompAtt, curCompRel, curCompPhat, curCompMix);
    compressorModule.process(passbandBuffer);

    // 8. Master Section: Blend with globalDryBuffer, apply master gain, safety clamp, compute atomic VU levels
    masterSection.process(passbandBuffer, globalDryBuffer, curMasterMix, curMasterGain);

    // 9. Copy back to output buffer
    for (int ch = 0; ch < 2; ++ch)
        buffer.copyFrom(ch, 0, passbandBuffer.getReadPointer(ch), numSamples);
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
