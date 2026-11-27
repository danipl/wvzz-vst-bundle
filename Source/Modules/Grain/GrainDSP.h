#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "GrainEngine.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace wvzz::modules {

/**
 * @brief Grain module: parameters (`grain_*`) plus a 32-voice GrainEngine.
 *
 * Two ways to use it:
 *  - Hosted: another module owns the delay lines and calls renderGrains() per sample
 *    (the Delay module in Granular mode). Call setExternalHost(true) before prepare().
 *  - Stand-alone: process(ctx) records the input into its own 1-second lines and replaces
 *    the signal with grains read 100 ms back (a granular pitch shifter).
 */
class GrainDSP : public IDSPModule
{
public:
    explicit GrainDSP(juce::String prefix = "grain");
    ~GrainDSP() override = default;

    [[nodiscard]] juce::String paramId(const juce::String& name) const
    {
        return instancePrefix.isEmpty() ? name : instancePrefix + "_" + name;
    }

    void registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    void bindParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() noexcept override;
    void process(ProcessContext& ctx) noexcept override;
    void randomize(juce::Random& rng) override;

    /** true: a host module owns the delay lines; stand-alone buffers are not allocated. */
    void setExternalHost(bool hosted) noexcept { externalHost = hosted; }

    /** Reads the parameters (and modulation offsets) once per block. Call before renderGrains(). */
    void updateBlock(const ProcessContext& ctx) noexcept;

    /** Renders one stereo grain sample from lines owned by the caller. */
    void renderGrains(const dsp::DelayLine& lineL, const dsp::DelayLine& lineR,
                      float baseDelayL, float baseDelayR, float& outL, float& outR) noexcept
    {
        engine.process(lineL, lineR, baseDelayL, baseDelayR, settings, outL, outR);
    }

    [[nodiscard]] int getNumActiveVoices() const noexcept { return engine.getNumActiveVoices(); }
    [[nodiscard]] float getGrainSizeSamples() const noexcept { return settings.sizeSamples; }

private:
    juce::String instancePrefix;
    double sampleRate { 44100.0 };
    bool externalHost { false };

    std::atomic<float>* sizeParam    { nullptr };
    std::atomic<float>* densityParam { nullptr };
    std::atomic<float>* pitchParam   { nullptr };
    std::atomic<float>* sprayParam   { nullptr };
    std::atomic<float>* reverseParam { nullptr };
    std::atomic<float>* spreadParam  { nullptr };

    dsp::GrainEngine engine;
    dsp::GrainEngine::Settings settings;

    // Stand-alone mode only
    dsp::DelayLine lineL;
    dsp::DelayLine lineR;
};

} // namespace wvzz::modules
