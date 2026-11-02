#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>

#include "Core/Interfaces/ProcessContext.h"
#include "Core/DSP/AudioVisualizerSource.h"
#include "Modules/BPFilter/BPFilterModule.h"
#include "Modules/Distortion/DistortionModule.h"
#include "Modules/MMFilter/MMFilterModule.h"
#include "Modules/Flanger/FlangerModule.h"
#include "Modules/MagicEQ/MagicEQModule.h"
#include "Modules/Compressor/CompressorModule.h"
#include "Modules/Modulation/ModulationModule.h"
#include "Modules/Master/MasterModule.h"
#include "Presets/FactoryPresets.h"

namespace wvzz {

class WvZzDistortionAudioProcessor : public juce::AudioProcessor
{
public:
    WvZzDistortionAudioProcessor();
    ~WvZzDistortionAudioProcessor() override;

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
    modules::BPFilterModule& getBPFilterModule() noexcept { return bpModule; }
    modules::DistortionModule& getDistortionModule() noexcept { return distModule; }
    modules::MMFilterModule& getMMFilterModule() noexcept { return mmModule; }
    modules::FlangerModule& getFlangerModule() noexcept { return flgModule; }
    modules::MagicEQModule& getMagicEQModule() noexcept { return meqModule; }
    modules::CompressorModule& getCompressorModule() noexcept { return compModule; }
    modules::ModulationModule& getModulationModule() noexcept { return modModule; }
    modules::MasterModule& getMasterModule() noexcept { return masterModule; }

    const modules::BPFilterModule& getBPFilterModule() const noexcept { return bpModule; }
    const modules::DistortionModule& getDistortionModule() const noexcept { return distModule; }
    const modules::MMFilterModule& getMMFilterModule() const noexcept { return mmModule; }
    const modules::FlangerModule& getFlangerModule() const noexcept { return flgModule; }
    const modules::MagicEQModule& getMagicEQModule() const noexcept { return meqModule; }
    const modules::CompressorModule& getCompressorModule() const noexcept { return compModule; }
    const modules::ModulationModule& getModulationModule() const noexcept { return modModule; }
    const modules::MasterModule& getMasterModule() const noexcept { return masterModule; }

    dsp::AudioVisualizerSource& getVisualizerSource() noexcept { return visualizerSource; }
    const dsp::AudioVisualizerSource& getVisualizerSource() const noexcept { return visualizerSource; }

    void randomizeParameters();

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createAggregatedLayout(
        modules::BPFilterModule& bp,
        modules::DistortionModule& dist,
        modules::MMFilterModule& mm,
        modules::FlangerModule& flg,
        modules::MagicEQModule& meq,
        modules::CompressorModule& comp,
        modules::ModulationModule& mod,
        modules::MasterModule& master);

    // 8 self-contained audio modules
    modules::BPFilterModule   bpModule;
    modules::DistortionModule distModule;
    modules::MMFilterModule   mmModule;
    modules::FlangerModule    flgModule;
    modules::MagicEQModule    meqModule;
    modules::CompressorModule compModule;
    modules::ModulationModule modModule;
    modules::MasterModule     masterModule;

    juce::AudioProcessorValueTreeState apvts;

    dsp::AudioVisualizerSource visualizerSource;
    int currentProgramIndex { 0 };

    // Pre-allocated scratch buffers for zero-allocation real-time routing
    juce::AudioBuffer<float> preBpInputBuffer;
    juce::AudioBuffer<float> bandRejectBuffer;
    juce::AudioBuffer<float> passbandBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WvZzDistortionAudioProcessor)
};

} // namespace wvzz
