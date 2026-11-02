#include "SignalComparisonComponent.h"
#include <cmath>

namespace wvzz::ui {

SignalComparisonComponent::SignalComparisonComponent(dsp::AudioVisualizerSource& source)
    : visualizerSource(source),
      fft(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    inputWaveform.resize(waveformBufferSize, 0.0f);
    outputWaveform.resize(waveformBufferSize, 0.0f);
    tempReadBuffer.resize(1024, 0.0f);

    smoothInputSpectrum.fill(-100.0f);
    smoothOutputSpectrum.fill(-100.0f);

    auto setupBtn = [this](juce::TextButton& btn) {
        btn.setColour(juce::TextButton::buttonColourId, WvZzLookAndFeel::cardBg);
        btn.setColour(juce::TextButton::buttonOnColourId, WvZzLookAndFeel::neonCyan.withAlpha(0.35f));
        btn.setColour(juce::TextButton::textColourOffId, WvZzLookAndFeel::textSecondary);
        btn.setColour(juce::TextButton::textColourOnId, WvZzLookAndFeel::textPrimary);
        addAndMakeVisible(btn);
    };

    setupBtn(scopeButton);
    setupBtn(spectrumButton);
    setupBtn(deltaButton);
    setupBtn(scaleButton);
    setupBtn(freezeButton);

    scopeButton.setClickingTogglesState(false);
    spectrumButton.setClickingTogglesState(false);
    deltaButton.setClickingTogglesState(false);

    scopeButton.onClick = [this] { setMode(Mode::Oscilloscope); };
    spectrumButton.onClick = [this] { setMode(Mode::Spectrum); };
    deltaButton.onClick = [this] { setMode(Mode::Delta); };

    scaleButton.onClick = [this] {
        if (gainScale >= 3.9f)
        {
            setGainScale(1.0f);
            scaleButton.setButtonText("1X");
        }
        else if (gainScale >= 1.9f)
        {
            setGainScale(4.0f);
            scaleButton.setButtonText("4X");
        }
        else
        {
            setGainScale(2.0f);
            scaleButton.setButtonText("2X");
        }
    };

    freezeButton.setClickingTogglesState(true);
    freezeButton.onClick = [this] {
        setFrozen(freezeButton.getToggleState());
    };

    startTimerHz(60);
}

SignalComparisonComponent::~SignalComparisonComponent()
{
    stopTimer();
}

void SignalComparisonComponent::setMode(Mode newMode)
{
    currentMode = newMode;
    repaint();
}

void SignalComparisonComponent::setFrozen(bool shouldFreeze)
{
    frozen = shouldFreeze;
    freezeButton.setToggleState(frozen, juce::dontSendNotification);
    freezeButton.setColour(juce::TextButton::buttonColourId,
        frozen ? juce::Colour(0xffe06c75).withAlpha(0.35f) : WvZzLookAndFeel::cardBg);
    repaint();
}

void SignalComparisonComponent::setGainScale(float scale)
{
    gainScale = juce::jlimit(0.5f, 10.0f, scale);
    repaint();
}

void SignalComparisonComponent::resized()
{
    auto bounds = getLocalBounds();
    auto topBar = bounds.removeFromTop(26).reduced(2, 2);

    scopeButton.setBounds(topBar.removeFromLeft(60));
    topBar.removeFromLeft(4);
    spectrumButton.setBounds(topBar.removeFromLeft(75));
    topBar.removeFromLeft(4);
    deltaButton.setBounds(topBar.removeFromLeft(60));

    freezeButton.setBounds(topBar.removeFromRight(60));
    topBar.removeFromRight(4);
    scaleButton.setBounds(topBar.removeFromRight(40));
}

void SignalComparisonComponent::timerCallback()
{
    if (!frozen)
    {
        // Drain samples from FIFOs into circular display waveforms
        int inReady = visualizerSource.getNumReadyInput();
        int outReady = visualizerSource.getNumReadyOutput();
        int toRead = std::min({ inReady, outReady, static_cast<int>(tempReadBuffer.size()) });

        if (toRead > 0)
        {
            visualizerSource.readInput(tempReadBuffer.data(), toRead);
            for (int i = 0; i < toRead; ++i)
            {
                inputWaveform[static_cast<size_t>((writeIndex + i) % waveformBufferSize)] = tempReadBuffer[static_cast<size_t>(i)];
            }

            visualizerSource.readOutput(tempReadBuffer.data(), toRead);
            for (int i = 0; i < toRead; ++i)
            {
                outputWaveform[static_cast<size_t>((writeIndex + i) % waveformBufferSize)] = tempReadBuffer[static_cast<size_t>(i)];
            }

            writeIndex = (writeIndex + toRead) % waveformBufferSize;
        }

        if (currentMode == Mode::Spectrum)
        {
            processFftData();
        }
    }

    repaint();
}

void SignalComparisonComponent::processFftData()
{
    // Extract the most recent fftSize samples
    int readStart = (writeIndex - fftSize + waveformBufferSize) % waveformBufferSize;
    for (int i = 0; i < fftSize; ++i)
    {
        int idx = (readStart + i) % waveformBufferSize;
        fftInputData[static_cast<size_t>(i)] = inputWaveform[static_cast<size_t>(idx)];
        fftOutputData[static_cast<size_t>(i)] = outputWaveform[static_cast<size_t>(idx)];
    }
    std::fill(fftInputData.begin() + fftSize, fftInputData.end(), 0.0f);
    std::fill(fftOutputData.begin() + fftSize, fftOutputData.end(), 0.0f);

    window.multiplyWithWindowingTable(fftInputData.data(), fftSize);
    window.multiplyWithWindowingTable(fftOutputData.data(), fftSize);

    fft.performFrequencyOnlyForwardTransform(fftInputData.data());
    fft.performFrequencyOnlyForwardTransform(fftOutputData.data());

    constexpr float minDb = -80.0f;
    constexpr float smoothing = 0.78f;

    for (size_t bin = 0; bin < fftSize / 2; ++bin)
    {
        float inMag = fftInputData[bin] / static_cast<float>(fftSize);
        float inDb = (inMag > 1e-6f) ? juce::Decibels::gainToDecibels(inMag) : minDb;
        smoothInputSpectrum[bin] = smoothInputSpectrum[bin] * smoothing + inDb * (1.0f - smoothing);

        float outMag = fftOutputData[bin] / static_cast<float>(fftSize);
        float outDb = (outMag > 1e-6f) ? juce::Decibels::gainToDecibels(outMag) : minDb;
        smoothOutputSpectrum[bin] = smoothOutputSpectrum[bin] * smoothing + outDb * (1.0f - smoothing);
    }
}

void SignalComparisonComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background track
    g.setColour(juce::Colour(0xff0d1117));
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(WvZzLookAndFeel::cardBorder);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    auto displayArea = bounds.withTrimmedTop(28.0f).reduced(4.0f);

    // Inner display screen
    g.setColour(juce::Colour(0xff090c10));
    g.fillRoundedRectangle(displayArea, 3.0f);
    g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.6f));
    g.drawRoundedRectangle(displayArea, 3.0f, 1.0f);

    // Update active tab buttons visual state
    scopeButton.setColour(juce::TextButton::buttonColourId,
        (currentMode == Mode::Oscilloscope) ? WvZzLookAndFeel::neonCyan.withAlpha(0.25f) : WvZzLookAndFeel::cardBg);
    spectrumButton.setColour(juce::TextButton::buttonColourId,
        (currentMode == Mode::Spectrum) ? WvZzLookAndFeel::neonCyan.withAlpha(0.25f) : WvZzLookAndFeel::cardBg);
    deltaButton.setColour(juce::TextButton::buttonColourId,
        (currentMode == Mode::Delta) ? WvZzLookAndFeel::neonAmber.withAlpha(0.25f) : WvZzLookAndFeel::cardBg);

    switch (currentMode)
    {
        case Mode::Oscilloscope: renderOscilloscope(g, displayArea); break;
        case Mode::Spectrum:     renderSpectrum(g, displayArea); break;
        case Mode::Delta:        renderDelta(g, displayArea); break;
    }

    // Legend at bottom
    auto legendArea = displayArea.removeFromBottom(16.0f).reduced(6.0f, 0.0f);
    g.setFont(juce::FontOptions(9.0f, juce::Font::bold));

    if (currentMode != Mode::Delta)
    {
        // Dry Input label
        g.setColour(WvZzLookAndFeel::neonCyan);
        g.drawText("--- IN (Dry)", legendArea.removeFromLeft(65.0f), juce::Justification::centredLeft, false);

        // Wet Output label
        g.setColour(WvZzLookAndFeel::neonAmber);
        g.drawText("___ OUT (Processed)", legendArea.removeFromLeft(110.0f), juce::Justification::centredLeft, false);
    }
    else
    {
        g.setColour(juce::Colour(0xffff2a85));
        g.drawText("--- DELTA (Output - Input Difference)", legendArea, juce::Justification::centredLeft, false);
    }
}

void SignalComparisonComponent::renderOscilloscope(juce::Graphics& g, juce::Rectangle<float> area)
{
    const float midY = area.getCentreY();
    const float halfHeight = area.getHeight() * 0.44f;

    // Grid center line
    g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.35f));
    g.drawHorizontalLine(static_cast<int>(midY), area.getX(), area.getRight());

    // Grid ±0.5 guidelines
    g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.18f));
    g.drawHorizontalLine(static_cast<int>(midY - halfHeight * 0.5f), area.getX(), area.getRight());
    g.drawHorizontalLine(static_cast<int>(midY + halfHeight * 0.5f), area.getX(), area.getRight());

    // Zero-crossing trigger search on input signal for stable view
    int triggerIdx = (writeIndex - 1024 + waveformBufferSize) % waveformBufferSize;
    for (int i = 0; i < 256; ++i)
    {
        int cur = (triggerIdx + i) % waveformBufferSize;
        int nxt = (cur + 1) % waveformBufferSize;
        if (inputWaveform[static_cast<size_t>(cur)] <= 0.0f && inputWaveform[static_cast<size_t>(nxt)] > 0.0f)
        {
            triggerIdx = cur;
            break;
        }
    }

    const int numPoints = static_cast<int>(area.getWidth());
    if (numPoints <= 1) return;

    juce::Path inPath;
    juce::Path outPath;

    for (int i = 0; i < numPoints; ++i)
    {
        float normX = static_cast<float>(i) / static_cast<float>(numPoints - 1);
        float x = area.getX() + normX * area.getWidth();

        int sampleIdx = (triggerIdx + (i * 2)) % waveformBufferSize;
        float inSample = juce::jlimit(-1.5f, 1.5f, inputWaveform[static_cast<size_t>(sampleIdx)] * gainScale);
        float outSample = juce::jlimit(-1.5f, 1.5f, outputWaveform[static_cast<size_t>(sampleIdx)] * gainScale);

        float inY = midY - inSample * halfHeight;
        float outY = midY - outSample * halfHeight;

        if (i == 0)
        {
            inPath.startNewSubPath(x, inY);
            outPath.startNewSubPath(x, outY);
        }
        else
        {
            inPath.lineTo(x, inY);
            outPath.lineTo(x, outY);
        }
    }

    // 1. Draw Input trace (Neon Cyan, dashed feel / thin)
    g.setColour(WvZzLookAndFeel::neonCyan.withAlpha(0.65f));
    g.strokePath(inPath, juce::PathStrokeType(1.2f));

    // 2. Draw Output trace (Neon Amber/Orange, vibrant)
    g.setColour(WvZzLookAndFeel::neonAmber);
    g.strokePath(outPath, juce::PathStrokeType(1.8f));
}

void SignalComparisonComponent::renderSpectrum(juce::Graphics& g, juce::Rectangle<float> area)
{
    // dB Reference Grid Lines
    constexpr float minDb = -72.0f;
    constexpr float maxDb = 6.0f;
    constexpr float dbRange = maxDb - minDb;

    auto dbToY = [&](float db) -> float {
        float norm = (db - minDb) / dbRange;
        return area.getBottom() - norm * area.getHeight();
    };

    g.setFont(juce::FontOptions(8.0f));
    for (float db : { 0.0f, -24.0f, -48.0f })
    {
        float y = dbToY(db);
        g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.25f));
        g.drawHorizontalLine(static_cast<int>(y), area.getX(), area.getRight());

        g.setColour(WvZzLookAndFeel::textSecondary.withAlpha(0.5f));
        g.drawText(juce::String(static_cast<int>(db)) + " dB",
                   static_cast<int>(area.getX()) + 3, static_cast<int>(y) - 9, 35, 9,
                   juce::Justification::left, false);
    }

    // Frequency Grid Lines: 100 Hz, 1 kHz, 10 kHz
    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 20000.0f;
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);

    auto freqToX = [&](float f) -> float {
        float norm = (std::log10(f) - logMin) / (logMax - logMin);
        return area.getX() + norm * area.getWidth();
    };

    for (float f : { 100.0f, 1000.0f, 10000.0f })
    {
        float x = freqToX(f);
        g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.2f));
        g.drawVerticalLine(static_cast<int>(x), area.getY(), area.getBottom());

        juce::String fStr = (f >= 1000.0f) ? (juce::String(static_cast<int>(f / 1000.0f)) + "k")
                                           : juce::String(static_cast<int>(f));
        g.setColour(WvZzLookAndFeel::textSecondary.withAlpha(0.5f));
        g.drawText(fStr, static_cast<int>(x) + 2, static_cast<int>(area.getY()) + 2, 25, 9,
                   juce::Justification::left, false);
    }

    // Render magnitude curves
    juce::Path inPath;
    juce::Path outPath;
    constexpr float sampleRate = 44100.0f;

    const int numBins = fftSize / 2;
    bool first = true;

    for (int bin = 1; bin < numBins; ++bin)
    {
        float freq = static_cast<float>(bin) * (sampleRate / static_cast<float>(fftSize));
        if (freq < minFreq) continue;
        if (freq > maxFreq) break;

        float x = freqToX(freq);
        float inY = juce::jlimit(area.getY(), area.getBottom(), dbToY(smoothInputSpectrum[static_cast<size_t>(bin)]));
        float outY = juce::jlimit(area.getY(), area.getBottom(), dbToY(smoothOutputSpectrum[static_cast<size_t>(bin)]));

        if (first)
        {
            inPath.startNewSubPath(x, inY);
            outPath.startNewSubPath(x, outY);
            first = false;
        }
        else
        {
            inPath.lineTo(x, inY);
            outPath.lineTo(x, outY);
        }
    }

    // Input spectrum path (Neon Cyan)
    g.setColour(WvZzLookAndFeel::neonCyan.withAlpha(0.55f));
    g.strokePath(inPath, juce::PathStrokeType(1.2f));

    // Output spectrum path (Neon Amber)
    g.setColour(WvZzLookAndFeel::neonAmber);
    g.strokePath(outPath, juce::PathStrokeType(1.8f));
}

void SignalComparisonComponent::renderDelta(juce::Graphics& g, juce::Rectangle<float> area)
{
    const float midY = area.getCentreY();
    const float halfHeight = area.getHeight() * 0.44f;

    // Center zero line
    g.setColour(WvZzLookAndFeel::cardBorder.withAlpha(0.35f));
    g.drawHorizontalLine(static_cast<int>(midY), area.getX(), area.getRight());

    int triggerIdx = (writeIndex - 1024 + waveformBufferSize) % waveformBufferSize;
    const int numPoints = static_cast<int>(area.getWidth());
    if (numPoints <= 1) return;

    juce::Path deltaPath;
    juce::Path fillPath;
    fillPath.startNewSubPath(area.getX(), midY);

    for (int i = 0; i < numPoints; ++i)
    {
        float normX = static_cast<float>(i) / static_cast<float>(numPoints - 1);
        float x = area.getX() + normX * area.getWidth();

        int sampleIdx = (triggerIdx + (i * 2)) % waveformBufferSize;
        float diff = (outputWaveform[static_cast<size_t>(sampleIdx)] - inputWaveform[static_cast<size_t>(sampleIdx)]) * gainScale;
        diff = juce::jlimit(-1.5f, 1.5f, diff);

        float y = midY - diff * halfHeight;

        if (i == 0)
            deltaPath.startNewSubPath(x, y);
        else
            deltaPath.lineTo(x, y);

        fillPath.lineTo(x, y);
    }

    fillPath.lineTo(area.getRight(), midY);
    fillPath.closeSubPath();

    // Fill difference area with translucent Magenta
    g.setColour(juce::Colour(0xffff2a85).withAlpha(0.2f));
    g.fillPath(fillPath);

    // Stroke difference line
    g.setColour(juce::Colour(0xffff2a85));
    g.strokePath(deltaPath, juce::PathStrokeType(1.8f));
}

} // namespace wvzz::ui
