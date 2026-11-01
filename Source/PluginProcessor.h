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
#include "DSP/ModulationEngine.h"
#include "Presets/FactoryPresets.h"

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
    camelphat::dsp::ModulationEngine& getModulationEngine() noexcept { return modEngine; }

    // Dice: Smart parameter randomizer
    void randomizeParameters();

private:
    void initSmoothers(double sampleRate) noexcept;
    void updateSmootherTargets() noexcept;

    juce::AudioProcessorValueTreeState apvts;
    camelphat::ParameterPointers paramPointers;

    // Core DSP Modules
    camelphat::dsp::BPFilter         bpFilter;
    camelphat::dsp::DistortionModule distortionModule;
    camelphat::dsp::MMFilter         mmFilter;
    camelphat::dsp::FlangerModule    flangerModule;
    camelphat::dsp::MagicEQ          magicEq;
    camelphat::dsp::CompressorModule compressorModule;
    camelphat::dsp::MasterSection    masterSection;
    camelphat::dsp::ModulationEngine modEngine;

    int currentProgramIndex { 0 };

    // Pre-allocated scratch buffers
    juce::AudioBuffer<float> preBpInputBuffer;
    juce::AudioBuffer<float> globalDryBuffer;
    juce::AudioBuffer<float> bandRejectBuffer;
    juce::AudioBuffer<float> passbandBuffer;

    // Parameter smoothing (10-20ms ramps)
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGainSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bpLowCutSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bpHighCutSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bpLowResSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bpHighResSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bpBrMixSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distMechSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distTubeSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distBitSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> distXcitaSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mmCutoffSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mmResSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mmMixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mmEnvDepthSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> flgRateSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> flgAmountSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> meqAmountSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> meqTuneSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> compAmountSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> compReleaseSmoothed;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> masterMixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> masterVolSmoothed;

    // bp_link tracking
    std::atomic<bool> isUpdatingLink { false };
    float lastLowCut { 20.0f };
    float lastHighCut { 20000.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CamelPhatAudioProcessor)
};
