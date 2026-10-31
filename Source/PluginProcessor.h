#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Parameters.h"
#include "DSP/IDSPModule.h"
#include "DSP/BPFilter.h"
#include "DSP/DistortionModule.h"
#include "DSP/MMFilter.h"
#include "DSP/FlangerModule.h"
#include "DSP/MagicEQ.h"
#include "DSP/CompressorModule.h"
#include "DSP/MasterSection.h"

class CamelPhatAudioProcessor : public juce::AudioProcessor
{
public:
    CamelPhatAudioProcessor();
    ~CamelPhatAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    const camelphat::ParameterPointers& getParameters() const noexcept { return paramPointers; }
    const camelphat::dsp::MasterSection& getMasterSection() const noexcept { return masterSection; }

private:
    void initSmoothers(double sampleRate) noexcept;
    void updateSmootherTargets() noexcept;

    juce::AudioProcessorValueTreeState apvts;
    camelphat::ParameterPointers paramPointers;

    // Core DSP Modules (SRP & ISP interfaces)
    camelphat::dsp::BPFilter         bpFilter;
    camelphat::dsp::DistortionModule distortionModule;
    camelphat::dsp::MMFilter         mmFilter;
    camelphat::dsp::FlangerModule    flangerModule;
    camelphat::dsp::MagicEQ          magicEq;
    camelphat::dsp::CompressorModule compressorModule;
    camelphat::dsp::MasterSection    masterSection;

    // Pre-allocated scratch buffers (zero heap allocations in processBlock)
    juce::AudioBuffer<float> globalDryBuffer;
    juce::AudioBuffer<float> bandRejectBuffer;
    juce::AudioBuffer<float> passbandBuffer;

    // Parameter smoothing to prevent zipper noise
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bpCutoffLowSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bpCutoffHighSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bpResonanceSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bpMixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bandRejectMixSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distTubeSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distMechSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distBitSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distXcitaSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distMixSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mmCutoffSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mmResonanceSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mmEnvAmountSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> flangerRateSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> flangerDepthSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> flangerFeedbackSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> flangerMixSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> magicEqAmountSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> magicEqFreqSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> compThresholdSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> compRatioSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> compMixSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> masterMixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> masterOutputGainSmoothed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CamelPhatAudioProcessor)
};
