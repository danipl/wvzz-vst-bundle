#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <algorithm>

namespace wvzz::dsp {

/**
 * @brief Lock-free dual-trace audio ring buffer source for real-time visualization.
 *
 * Captures raw input and final output audio streams with strictly zero memory allocations
 * and no mutex locks on the audio thread using juce::AbstractFifo.
 */
class AudioVisualizerSource
{
public:
    static constexpr int defaultCapacity = 16384;

    explicit AudioVisualizerSource(int capacity = defaultCapacity)
        : bufferCapacity(capacity),
          inputFifo(capacity),
          outputFifo(capacity)
    {
        inputBuffer.resize(static_cast<size_t>(capacity), 0.0f);
        outputBuffer.resize(static_cast<size_t>(capacity), 0.0f);
    }

    ~AudioVisualizerSource() = default;

    void prepare(double /*sampleRate*/, int /*maxBlockSize*/)
    {
        reset();
    }

    void reset() noexcept
    {
        inputFifo.reset();
        outputFifo.reset();
        std::fill(inputBuffer.begin(), inputBuffer.end(), 0.0f);
        std::fill(outputBuffer.begin(), outputBuffer.end(), 0.0f);
    }

    /**
     * @brief Pushes input audio samples into the lock-free input ring buffer.
     * Real-time audio thread safe: 0 heap allocations, lock-free.
     */
    void pushInput(const float* const* channelData, int numChannels, int numSamples) noexcept
    {
        pushToFifo(inputFifo, inputBuffer, channelData, numChannels, numSamples);
    }

    /**
     * @brief Pushes output audio samples into the lock-free output ring buffer.
     * Real-time audio thread safe: 0 heap allocations, lock-free.
     */
    void pushOutput(const float* const* channelData, int numChannels, int numSamples) noexcept
    {
        pushToFifo(outputFifo, outputBuffer, channelData, numChannels, numSamples);
    }

    /**
     * @brief Reads available input samples on the GUI thread.
     */
    int readInput(float* dest, int maxSamples) noexcept
    {
        return readFromFifo(inputFifo, inputBuffer, dest, maxSamples);
    }

    /**
     * @brief Reads available output samples on the GUI thread.
     */
    int readOutput(float* dest, int maxSamples) noexcept
    {
        return readFromFifo(outputFifo, outputBuffer, dest, maxSamples);
    }

    [[nodiscard]] int getNumReadyInput() const noexcept
    {
        return inputFifo.getNumReady();
    }

    [[nodiscard]] int getNumReadyOutput() const noexcept
    {
        return outputFifo.getNumReady();
    }

    [[nodiscard]] int getCapacity() const noexcept
    {
        return bufferCapacity;
    }

private:
    void pushToFifo(juce::AbstractFifo& fifo, std::vector<float>& buffer,
                    const float* const* channelData, int numChannels, int numSamples) noexcept
    {
        if (numSamples <= 0 || channelData == nullptr || channelData[0] == nullptr)
            return;

        int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
        fifo.prepareToWrite(numSamples, start1, size1, start2, size2);

        auto writeChunk = [&](int srcOffset, int destStart, int count) noexcept {
            float* dest = buffer.data() + destStart;
            if (numChannels >= 2 && channelData[1] != nullptr)
            {
                const float* ch0 = channelData[0] + srcOffset;
                const float* ch1 = channelData[1] + srcOffset;
                for (int i = 0; i < count; ++i)
                    dest[i] = 0.5f * (ch0[i] + ch1[i]);
            }
            else
            {
                const float* ch0 = channelData[0] + srcOffset;
                std::copy(ch0, ch0 + count, dest);
            }
        };

        if (size1 > 0)
            writeChunk(0, start1, size1);
        if (size2 > 0)
            writeChunk(size1, start2, size2);

        fifo.finishedWrite(size1 + size2);
    }

    int readFromFifo(juce::AbstractFifo& fifo, const std::vector<float>& buffer,
                     float* dest, int maxSamples) noexcept
    {
        if (maxSamples <= 0 || dest == nullptr)
            return 0;

        int numReady = fifo.getNumReady();
        int toRead = std::min(numReady, maxSamples);
        if (toRead <= 0)
            return 0;

        int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
        fifo.prepareToRead(toRead, start1, size1, start2, size2);

        if (size1 > 0)
            std::copy(buffer.data() + start1, buffer.data() + start1 + size1, dest);
        if (size2 > 0)
            std::copy(buffer.data() + start2, buffer.data() + start2 + size2, dest + size1);

        fifo.finishedRead(size1 + size2);
        return size1 + size2;
    }

    const int bufferCapacity;
    juce::AbstractFifo inputFifo;
    juce::AbstractFifo outputFifo;
    std::vector<float> inputBuffer;
    std::vector<float> outputBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioVisualizerSource)
};

} // namespace wvzz::dsp
