#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>

#include "Core/Interfaces/ProcessContext.h"
#include "Core/DSP/AudioVisualizerSource.h"
#include "Modules/Chorus/ChorusModule.h"
#include "Modules/Phaser/PhaserModule.h"
#include "Modules/Flanger/FlangerModule.h"
#include "Modules/Tremolo/TremoloModule.h"
#include "Modules/Modulation/ModulationModule.h"
#include "Modules/Master/MasterModule.h"
#include "Presets/FactoryPresets.h"

namespace wvzz {

/**
 * @brief WvZz Modulation: fixed serial chain Chorus -> Phaser -> Flanger -> Tremolo -> Master.
 */
class WvZzModulationAudioProcessor : public juce::AudioProcessor
{
public:
    /** Tail reported to the host. Covers the longest feedback decay (flanger at 0.95 feedback). */
    static constexpr double tailLengthSeconds = 2.0;

    WvZzModulationAudioProcessor();
    ~WvZzModulationAudioProcessor() override;

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
    modules::ChorusModule& getChorusModule() noexcept { return chorusModule; }
    modules::PhaserModule& getPhaserModule() noexcept { return phaserModule; }
    modules::FlangerModule& getFlangerModule() noexcept { return flangerModule; }
    modules::TremoloModule& getTremoloModule() noexcept { return tremoloModule; }
    modules::ModulationModule& getModulationModule() noexcept { return modModule; }
    modules::MasterModule& getMasterModule() noexcept { return masterModule; }

    dsp::AudioVisualizerSource& getVisualizerSource() noexcept { return visualizerSource; }
    const dsp::AudioVisualizerSource& getVisualizerSource() const noexcept { return visualizerSource; }

    /** DICE: sets the effect knobs to random, musically safe values (message thread). */
    void randomizeParameters();

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createAggregatedLayout(
        modules::ChorusModule& chorus,
        modules::PhaserModule& phaser,
        modules::FlangerModule& flanger,
        modules::TremoloModule& tremolo,
        modules::ModulationModule& mod,
        modules::MasterModule& master);

    modules::ChorusModule     chorusModule;
    modules::PhaserModule     phaserModule;
    modules::FlangerModule    flangerModule;
    modules::TremoloModule    tremoloModule;
    modules::ModulationModule modModule;
    modules::MasterModule     masterModule;

    juce::AudioProcessorValueTreeState apvts;

    dsp::AudioVisualizerSource visualizerSource;
    int currentProgramIndex { 0 };

    // Pre-allocated scratch buffers for zero-allocation real-time routing
    juce::AudioBuffer<float> dryInputBuffer;
    juce::AudioBuffer<float> auxBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WvZzModulationAudioProcessor)
};

} // namespace wvzz
