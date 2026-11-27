#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <memory>
#include <vector>

#include "Core/Interfaces/ProcessContext.h"
#include "Core/DSP/AudioVisualizerSource.h"
#include "Modules/Delay/DelayModule.h"
#include "Modules/Grain/GrainModule.h"
#include "Modules/Modulation/ModulationModule.h"
#include "Modules/Master/MasterModule.h"
#include "Presets/FactoryPresets.h"

namespace wvzz {

/**
 * @brief WvZz Delay: Delay (Clean, Tape, BBD, Granular via the Grain module) -> Master.
 */
class WvZzDelayAudioProcessor : public juce::AudioProcessor
{
public:
    WvZzDelayAudioProcessor();
    ~WvZzDelayAudioProcessor() override;

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
    modules::DelayModule& getDelayModule() noexcept { return delayModule; }
    modules::GrainModule& getGrainModule() noexcept { return grainModule; }
    modules::ModulationModule& getModulationModule() noexcept { return modModule; }
    modules::MasterModule& getMasterModule() noexcept { return masterModule; }

    dsp::AudioVisualizerSource& getVisualizerSource() noexcept { return visualizerSource; }
    const dsp::AudioVisualizerSource& getVisualizerSource() const noexcept { return visualizerSource; }

    /** DICE: sets the effect knobs to random, musically safe values (message thread). */
    void randomizeParameters();

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createAggregatedLayout(
        modules::DelayModule& delay,
        modules::GrainModule& grain,
        modules::ModulationModule& mod,
        modules::MasterModule& master);

    modules::DelayModule      delayModule;
    modules::GrainModule      grainModule;
    modules::ModulationModule modModule;
    modules::MasterModule     masterModule;

    juce::AudioProcessorValueTreeState apvts;

    dsp::AudioVisualizerSource visualizerSource;
    int currentProgramIndex { 0 };
    std::atomic<double> lastHostBpm { 120.0 }; // for the tempo-synced tail length

    // Pre-allocated scratch buffers for zero-allocation real-time routing
    juce::AudioBuffer<float> dryInputBuffer;
    juce::AudioBuffer<float> auxBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WvZzDelayAudioProcessor)
};

} // namespace wvzz
