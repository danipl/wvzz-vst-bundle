#pragma once

#include "../../Core/Interfaces/IDSPModule.h"
#include "../../Core/Interfaces/ProcessContext.h"
#include "DelayLine.h"
#include "DelayTapState.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <vector>
#include <cmath>

namespace wvzz::modules {

class GrainDSP;

inline const juce::StringArray& getDelayModeNames()
{
    static const juce::StringArray names { "Clean", "Tape", "BBD", "Granular" };
    return names;
}

inline const juce::StringArray& getDelaySyncDivNames()
{
    static const juce::StringArray names {
        "1/1", "1/2D", "1/2", "1/2T", "1/4D", "1/4", "1/4T",
        "1/8D", "1/8", "1/8T", "1/16D", "1/16", "1/16T", "1/32"
    };
    return names;
}

/**
 * @brief Stereo delay with four modes (Clean, Tape, BBD, Granular), filtered feedback up to 110%,
 * all-pass diffusion, ducking, and a bypass that lets the tail fade out.
 * Headless and real-time safe: every buffer is sized for 4 seconds in prepare().
 */
class DelayDSP : public IDSPModule
{
public:
    enum Mode : int { Clean = 0, Tape = 1, BBD = 2, Granular = 3 };
    enum Routing : int { Stereo = 0, PingPong = 1 };

    static constexpr float maxTimeMs = 4000.0f;
    static constexpr float minTimeMs = 20.0f;
    static constexpr float crossfadeMs = 50.0f;
    static constexpr float maxGlideStep = 0.35f; // Tape / BBD: largest change of the delay per sample

    explicit DelayDSP(juce::String prefix = "dly");
    ~DelayDSP() override = default;

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

    /** The grain module used for Granular mode. Without one, Granular mode falls back to Clean. */
    void setGrainSource(GrainDSP* grain) noexcept { grainSource = grain; }

    /** Length of one note value in quarter-note beats for a `sync_div` choice index. */
    [[nodiscard]] static double beatsForSyncDiv(int index) noexcept;

    /** Delay time in ms from the current parameters (free or tempo-synced), 20..4000. */
    [[nodiscard]] float getTargetTimeMs(double hostBpm) const noexcept;

    /** Seconds until the repeats fall by 60 dB for the current time and feedback, in [time, 60]. */
    [[nodiscard]] double getTailSeconds(double hostBpm = 120.0) const noexcept;

    [[nodiscard]] const dsp::DelayTapState& getTapState() const noexcept { return tapState; }

private:
    /** Schroeder all-pass with a fixed whole-sample length. At gain 0 it is a pure delay. */
    struct AllPass
    {
        std::vector<float> buffer;
        int index { 0 };

        void prepare(int lengthSamples) { buffer.assign(static_cast<size_t>(juce::jmax(1, lengthSamples)), 0.0f); index = 0; }
        void clear() noexcept { std::fill(buffer.begin(), buffer.end(), 0.0f); index = 0; }
        [[nodiscard]] int length() const noexcept { return static_cast<int>(buffer.size()); }

        float process(float x, float g) noexcept
        {
            const float delayed = buffer[static_cast<size_t>(index)];
            float w = x + g * delayed;
            if (std::abs(w) < 1.0e-15f)
                w = 0.0f;
            buffer[static_cast<size_t>(index)] = w;
            if (++index >= length())
                index = 0;
            return delayed - g * w;
        }
    };

    /** Per-channel state of the delay loop. */
    struct Channel
    {
        dsp::DelayLine line;
        std::array<AllPass, 2> diffusers;
        int diffuserLength { 0 };       // total samples of both all-pass sections

        // Clean mode: two taps and a crossfade between them
        float tapFrom { 0.0f };
        float tapTo { 0.0f };
        float fadePosition { 1.0f };    // 0..1, 1 = fade finished

        // Tape / BBD mode: gliding tap
        float glideDelay { 0.0f };

        // Loop filters
        float hpState { 0.0f };
        float lpState { 0.0f };
        float bbdState1 { 0.0f };
        float bbdState2 { 0.0f };

        float feedback { 0.0f };        // last wet sample, fed back into the line

        void clear() noexcept;
    };

    [[nodiscard]] float readTap(Channel& ch, float targetDelay, int mode, float wowOffset, float fadeStep, float glideCoeff) noexcept;

    juce::String instancePrefix;
    double sampleRate { 44100.0 };
    GrainDSP* grainSource { nullptr };

    std::atomic<float>* bypassParam    { nullptr };
    std::atomic<float>* modeParam      { nullptr };
    std::atomic<float>* syncParam      { nullptr };
    std::atomic<float>* timeParam      { nullptr };
    std::atomic<float>* syncDivParam   { nullptr };
    std::atomic<float>* routingParam   { nullptr };
    std::atomic<float>* offsetParam    { nullptr };
    std::atomic<float>* feedbackParam  { nullptr };
    std::atomic<float>* hpParam        { nullptr };
    std::atomic<float>* lpParam        { nullptr };
    std::atomic<float>* diffusionParam { nullptr };
    std::atomic<float>* duckParam      { nullptr };
    std::atomic<float>* wowParam       { nullptr };
    std::atomic<float>* driveParam     { nullptr };
    std::atomic<float>* mixParam       { nullptr };

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> feedbackSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> diffusionSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sendSmoothed;   // 1 = record input, 0 = bypassed

    std::array<Channel, 2> channels;

    double wowPhase { 0.0 };
    double flutterPhase { 0.0 };
    float duckEnvelope { 0.0f };
    int lastMode { Clean };

    dsp::DelayTapState tapState;
};

} // namespace wvzz::modules
