#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "../DSP/AudioVisualizerSource.h"
#include "CamelPhatLookAndFeel.h"
#include <vector>
#include <array>

namespace camelphat::ui {

/**
 * @brief High-performance dual-trace comparative visualizer.
 * Supports Oscilloscope (time-domain), Spectrum Analyzer (FFT frequency-domain),
 * and Delta (difference) modes with hardware-accelerated vector drawing.
 */
class SignalComparisonComponent : public juce::Component,
                                  private juce::Timer
{
public:
    enum class Mode
    {
        Oscilloscope = 0,
        Spectrum,
        Delta
    };

    explicit SignalComparisonComponent(dsp::AudioVisualizerSource& source);
    ~SignalComparisonComponent() override;

    void setMode(Mode newMode);
    [[nodiscard]] Mode getMode() const noexcept { return currentMode; }

    void setFrozen(bool shouldFreeze);
    [[nodiscard]] bool isFrozen() const noexcept { return frozen; }

    void setGainScale(float scale);
    [[nodiscard]] float getGainScale() const noexcept { return gainScale; }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    void renderOscilloscope(juce::Graphics& g, juce::Rectangle<float> area);
    void renderSpectrum(juce::Graphics& g, juce::Rectangle<float> area);
    void renderDelta(juce::Graphics& g, juce::Rectangle<float> area);

    void processFftData();

    dsp::AudioVisualizerSource& visualizerSource;

    Mode currentMode { Mode::Oscilloscope };
    bool frozen { false };
    float gainScale { 1.0f };

    // Header buttons
    juce::TextButton scopeButton { "SCOPE" };
    juce::TextButton spectrumButton { "SPECTRUM" };
    juce::TextButton deltaButton { "DELTA" };
    juce::TextButton scaleButton { "1X" };
    juce::TextButton freezeButton { "FREEZE" };

    // Rolling waveform history buffer (2048 points)
    static constexpr int waveformBufferSize = 2048;
    std::vector<float> inputWaveform;
    std::vector<float> outputWaveform;
    int writeIndex { 0 };

    // Temporary read scratch buffer
    std::vector<float> tempReadBuffer;

    // FFT Engine (Order 10 = 1024 points)
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder; // 1024
    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize * 2> fftInputData {};
    std::array<float, fftSize * 2> fftOutputData {};
    std::array<float, fftSize / 2> smoothInputSpectrum {};
    std::array<float, fftSize / 2> smoothOutputSpectrum {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalComparisonComponent)
};

} // namespace camelphat::ui
