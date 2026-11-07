#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>

#include "Core/Interfaces/ProcessContext.h"
#include "Core/DSP/AudioVisualizerSource.h"
#include "Modules/DynDetector/DynDetectorModule.h"
#include "Modules/TransientShaper/TransientShaperModule.h"
#include "Modules/MultibandOTT/MultibandOTTModule.h"
#include "Modules/Compressor/CompressorModule.h"
#include "Modules/RhythmicPumper/RhythmicPumperModule.h"
#include "Modules/MaximizerClipper/MaximizerClipperModule.h"
#include "Modules/Modulation/ModulationModule.h"
#include "Modules/Master/MasterModule.h"
#include "Presets/FactoryPresets.h"

namespace wvzz {

class WvZzDynamicsAudioProcessor : public juce::AudioProcessor
{
public:
    WvZzDynamicsAudioProcessor();
    ~WvZzDynamicsAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) noexcept override;

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
    const juce::AudioProcessorValueTreeState& getAPVTS() const noexcept { return apvts; }

    // Module accessors
    modules::DynDetectorModule& getDetectorModule() noexcept { return detModule; }
    modules::TransientShaperModule& getTransientModule() noexcept { return transModule; }
    modules::MultibandOTTModule& getOTTModule() noexcept { return ottModule; }
    modules::CompressorModule& getCompressorModule() noexcept { return compModule; }
    modules::RhythmicPumperModule& getPumperModule() noexcept { return pumpModule; }
    modules::MaximizerClipperModule& getClipperModule() noexcept { return clipModule; }
    modules::ModulationModule& getModulationModule() noexcept { return modModule; }
    modules::MasterModule& getMasterModule() noexcept { return masterModule; }

    dsp::AudioVisualizerSource& getVisualizerSource() noexcept { return visualizerSource; }
    const dsp::AudioVisualizerSource& getVisualizerSource() const noexcept { return visualizerSource; }

    void randomizeParameters();

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createAggregatedLayout(
        modules::DynDetectorModule& det,
        modules::TransientShaperModule& trans,
        modules::MultibandOTTModule& ott,
        modules::CompressorModule& comp,
        modules::RhythmicPumperModule& pump,
        modules::MaximizerClipperModule& clip,
        modules::ModulationModule& mod,
        modules::MasterModule& master);

    // 8 self-contained audio modules
    modules::DynDetectorModule      detModule;
    modules::TransientShaperModule  transModule;
    modules::MultibandOTTModule     ottModule;
    modules::CompressorModule       compModule;
    modules::RhythmicPumperModule   pumpModule;
    modules::MaximizerClipperModule clipModule;
    modules::ModulationModule       modModule;
    modules::MasterModule           masterModule;

    juce::AudioProcessorValueTreeState apvts;

    dsp::AudioVisualizerSource visualizerSource;
    int currentProgramIndex { 0 };

    // Pre-allocated scratch buffers for zero-allocation real-time routing
    juce::AudioBuffer<float> dryInputBuffer;
    juce::AudioBuffer<float> detectorSidechainBuffer;
    juce::AudioBuffer<float> subAuxBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WvZzDynamicsAudioProcessor)
};

} // namespace wvzz
