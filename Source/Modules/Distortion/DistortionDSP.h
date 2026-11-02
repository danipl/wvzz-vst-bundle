#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace wvzz::modules {

class DistortionDSP : public IDSPModule
{
public:
    explicit DistortionDSP(juce::String prefix = "dist");
    ~DistortionDSP() override = default;

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

    [[nodiscard]] int getLatencyInSamples() const noexcept
    {
        return static_cast<int>(oversampling.getLatencyInSamples());
    }

    // Direct setters for testing
    void setParameters(float mechNorm, float tubeNorm, float bitNorm, float xcitaNorm, bool bypass = false) noexcept;

private:
    juce::String instancePrefix;
    double baseSampleRate { 44100.0 };

    std::atomic<float>* bypassParam { nullptr };
    std::atomic<float>* mechParam   { nullptr };
    std::atomic<float>* tubeParam   { nullptr };
    std::atomic<float>* bitParam    { nullptr };
    std::atomic<float>* xcitaParam  { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mechSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> tubeSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bitSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> xcitaSmoothed;

    juce::dsp::Oversampling<float> oversampling;

    // Mech filters (oversampled rate)
    juce::dsp::IIR::Filter<float> mechPreFilterL;
    juce::dsp::IIR::Filter<float> mechPreFilterR;
    juce::dsp::IIR::Filter<float> mechPostFilterL;
    juce::dsp::IIR::Filter<float> mechPostFilterR;

    // Tube DC blocker (oversampled rate)
    juce::dsp::IIR::Filter<float> tubeDcBlockerL;
    juce::dsp::IIR::Filter<float> tubeDcBlockerR;

    // Xcita filters (oversampled rate)
    juce::dsp::IIR::Filter<float> xcitaHpfL;
    juce::dsp::IIR::Filter<float> xcitaHpfR;
    juce::dsp::IIR::Filter<float> xcitaDcBlockerL;
    juce::dsp::IIR::Filter<float> xcitaDcBlockerR;

    // Bitcrusher state (base rate)
    float bitHoldSampleL { 0.0f };
    float bitHoldSampleR { 0.0f };
    int   bitHoldCounterL { 0 };
    int   bitHoldCounterR { 0 };
};

} // namespace wvzz::modules
