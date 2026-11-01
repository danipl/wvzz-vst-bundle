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
#include "DSP/AudioVisualizerSource.h"
#include "Presets/FactoryPresets.h"

class InputMeterSource : public camelphat::dsp::IMeterSource
{
public:
    InputMeterSource() = default;
    ~InputMeterSource() override = default;

    void reset() noexcept
    {
        peakLeft.store(0.0f, std::memory_order_relaxed);
        peakRight.store(0.0f, std::memory_order_relaxed);
        rmsLeft.store(0.0f, std::memory_order_relaxed);
        rmsRight.store(0.0f, std::memory_order_relaxed);
    }

    void update(const juce::AudioBuffer<float>& buffer, int numSamples) noexcept
    {
        if (numSamples <= 0 || buffer.getNumChannels() == 0)
            return;

        const float* inL = buffer.getReadPointer(0);
        const float* inR = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : inL;

        float blockPeakL = 0.0f;
        float blockPeakR = 0.0f;
        float blockSumSqL = 0.0f;
        float blockSumSqR = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            const float sL = inL[i];
            const float sR = inR[i];
            const float aL = std::abs(sL);
            const float aR = std::abs(sR);
            blockPeakL = juce::jmax(blockPeakL, aL);
            blockPeakR = juce::jmax(blockPeakR, aR);
            blockSumSqL += sL * sL;
            blockSumSqR += sR * sR;
        }

        const float blockRmsL = std::sqrt(blockSumSqL / static_cast<float>(numSamples));
        const float blockRmsR = std::sqrt(blockSumSqR / static_cast<float>(numSamples));

        updateMeter(peakLeft, blockPeakL);
        updateMeter(peakRight, blockPeakR);
        updateMeter(rmsLeft, blockRmsL);
        updateMeter(rmsRight, blockRmsR);
    }

    [[nodiscard]] float getPeakLevel(int channel) const noexcept override
    {
        return (channel == 0) ? peakLeft.load(std::memory_order_relaxed)
                               : peakRight.load(std::memory_order_relaxed);
    }

    [[nodiscard]] float getRmsLevel(int channel) const noexcept override
    {
        return (channel == 0) ? rmsLeft.load(std::memory_order_relaxed)
                               : rmsRight.load(std::memory_order_relaxed);
    }

private:
    static inline void updateMeter(std::atomic<float>& target, float newBlockValue) noexcept
    {
        const float current = target.load(std::memory_order_relaxed);
        const float updated = (newBlockValue > current) ? newBlockValue : (current * 0.85f + newBlockValue * 0.15f);
        target.store(updated, std::memory_order_relaxed);
    }

    std::atomic<float> peakLeft  { 0.0f };
    std::atomic<float> peakRight { 0.0f };
    std::atomic<float> rmsLeft   { 0.0f };
    std::atomic<float> rmsRight  { 0.0f };
};

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
    const camelphat::dsp::IMeterSource& getInputMeterSource() const noexcept { return inputMeterSource; }
    const camelphat::dsp::CompressorModule& getCompressorModule() const noexcept { return compressorModule; }
    camelphat::dsp::ModulationEngine& getModulationEngine() noexcept { return modEngine; }
    camelphat::dsp::AudioVisualizerSource& getVisualizerSource() noexcept { return visualizerSource; }
    const camelphat::dsp::AudioVisualizerSource& getVisualizerSource() const noexcept { return visualizerSource; }

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
    InputMeterSource                 inputMeterSource;
    camelphat::dsp::AudioVisualizerSource visualizerSource;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CamelPhatAudioProcessor)
};
