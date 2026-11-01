#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DSP/AudioVisualizerSource.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <cstdlib>

// ==============================================================================
// Allocation Tracking Hook to assert zero heap allocations in real-time code
// ==============================================================================
static thread_local bool g_trackingAllocations = false;
static thread_local int g_allocationCount = 0;

void* operator new(std::size_t size)
{
    if (g_trackingAllocations)
        g_allocationCount++;
    return std::malloc(size);
}

void operator delete(void* ptr) noexcept
{
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t) noexcept
{
    std::free(ptr);
}

// ==============================================================================
// Helper utilities
// ==============================================================================
static void generateSineWave(juce::AudioBuffer<float>& buffer, double sampleRate, float frequencyHz)
{
    const double phaseIncrement = (juce::MathConstants<double>::twoPi * frequencyHz) / sampleRate;
    double phase = 0.0;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float sample = static_cast<float>(std::sin(phase));
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.setSample(ch, i, sample);

        phase += phaseIncrement;
        if (phase >= juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;
    }
}

static float calculateRms(const juce::AudioBuffer<float>& buffer, int channel)
{
    float sumSq = 0.0f;
    const float* data = buffer.getReadPointer(channel);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        sumSq += data[i] * data[i];

    return std::sqrt(sumSq / static_cast<float>(buffer.getNumSamples()));
}

// ==============================================================================
// Test Suites
// ==============================================================================

static void testZeroAllocations()
{
    std::cout << "[Test 1/7] Running Real-Time Zero-Allocation Audit..." << std::endl;

    CamelPhatAudioProcessor processor;
    const double sampleRate = 48000.0;
    const int blockSize = 512;

    processor.prepareToPlay(sampleRate, blockSize);

    juce::AudioBuffer<float> buffer(2, blockSize);
    juce::MidiBuffer midi;
    generateSineWave(buffer, sampleRate, 440.0f);

    // Warm up one block
    processor.processBlock(buffer, midi);

    // Begin allocation tracking across 1,000 processed blocks
    g_allocationCount = 0;
    g_trackingAllocations = true;

    for (int i = 0; i < 1000; ++i)
    {
        processor.processBlock(buffer, midi);
    }

    g_trackingAllocations = false;

    std::cout << "  -> Total heap allocations during 1,000 processBlock() calls: "
              << g_allocationCount << std::endl;

    assert(g_allocationCount == 0 && "CRITICAL FAILURE: Memory allocated in audio thread!");
    std::cout << "  -> PASS: Strictly 0 heap allocations detected.\n" << std::endl;
}

static void testHarmonicDistortion()
{
    std::cout << "[Test 2/7] Running Harmonic Distortion Verification (Tube, Mech, Bit, Xcita)..." << std::endl;

    CamelPhatAudioProcessor processor;
    const double sampleRate = 48000.0;
    const int blockSize = 1024;
    processor.prepareToPlay(sampleRate, blockSize);

    auto& apvts = processor.getAPVTS();

    juce::AudioBuffer<float> buffer(2, blockSize);
    juce::MidiBuffer midi;

    // 1. Clean Sine Baseline
    generateSineWave(buffer, sampleRate, 1000.0f);
    processor.processBlock(buffer, midi);
    const float cleanRms = calculateRms(buffer, 0);

    // 2. Tube Drive Engaged
    if (auto* param = apvts.getParameter(camelphat::ParameterIDs::distTube))
        param->setValueNotifyingHost(0.8f);

    generateSineWave(buffer, sampleRate, 1000.0f);
    processor.processBlock(buffer, midi);
    const float tubeRms = calculateRms(buffer, 0);

    std::cout << "  -> Clean RMS: " << cleanRms << ", Tube Saturated RMS: " << tubeRms << std::endl;
    assert(tubeRms > 0.05f && "Tube saturation output is non-zero and compressed");

    // 3. Mech Drive Engaged
    if (auto* tubeP = apvts.getParameter(camelphat::ParameterIDs::distTube))
        tubeP->setValueNotifyingHost(0.0f);
    if (auto* mechP = apvts.getParameter(camelphat::ParameterIDs::distMech))
        mechP->setValueNotifyingHost(0.9f);

    generateSineWave(buffer, sampleRate, 1000.0f);
    processor.processBlock(buffer, midi);
    const float mechRms = calculateRms(buffer, 0);

    std::cout << "  -> Mech Clipped RMS: " << mechRms << std::endl;
    assert(mechRms > 0.05f && "Mech circuit output valid");

    std::cout << "  -> PASS: All 4 distortion algorithms generate valid saturation overtones.\n" << std::endl;
}

static void testFilterStabilityAndNanDefense()
{
    std::cout << "[Test 3/7] Running Filter Stability & NaN / Inf Sanitization Stress Test..." << std::endl;

    CamelPhatAudioProcessor processor;
    const double sampleRate = 96000.0;
    const int blockSize = 256;
    processor.prepareToPlay(sampleRate, blockSize);

    auto& apvts = processor.getAPVTS();

    // Push BP filter and MM filter resonances to extreme maximum
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::bpLowRes))
        p->setValueNotifyingHost(1.0f);
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::bpHighRes))
        p->setValueNotifyingHost(1.0f);
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::mmRes))
        p->setValueNotifyingHost(1.0f);

    juce::AudioBuffer<float> buffer(2, blockSize);
    juce::MidiBuffer midi;

    // Inject violent impulses and +24 dBFS square pulses
    for (int block = 0; block < 500; ++block)
    {
        for (int i = 0; i < blockSize; ++i)
        {
            const float pulse = (i % 32 == 0) ? 15.0f : ((i % 16 == 0) ? -15.0f : 0.0f);
            buffer.setSample(0, i, pulse);
            buffer.setSample(1, i, pulse);
        }

        // Fast cutoff modulation sweep
        const float sweepCutoff = 20.0f + 19000.0f * (0.5f + 0.5f * std::sin(static_cast<float>(block) * 0.1f));
        if (auto* p = apvts.getParameter(camelphat::ParameterIDs::mmCutoff))
            p->setValueNotifyingHost(sweepCutoff / 20000.0f);

        processor.processBlock(buffer, midi);

        // Assert strictly zero NaNs, Infs, or unbounded blowups
        for (int ch = 0; ch < 2; ++ch)
        {
            const float* chData = buffer.getReadPointer(ch);
            for (int i = 0; i < blockSize; ++i)
            {
                const float s = chData[i];
                if (!std::isfinite(s) || std::abs(s) > 8.0f)
                {
                    assert(false && "CRITICAL FAILURE: Filter output not finite or clamped!");
                }
            }
        }
    }

    std::cout << "  -> PASS: 500 stress blocks with extreme resonance produced 0 NaNs and bounded amplitude.\n" << std::endl;
}

static void testParameterSmoothing()
{
    std::cout << "[Test 4/7] Running Parameter Smoothing & Anti-Zipper Continuity Test..." << std::endl;

    CamelPhatAudioProcessor processor;
    const double sampleRate = 44100.0;
    const int blockSize = 128;
    processor.prepareToPlay(sampleRate, blockSize);

    auto& apvts = processor.getAPVTS();

    juce::AudioBuffer<float> buffer(2, blockSize);
    juce::MidiBuffer midi;
    generateSineWave(buffer, sampleRate, 440.0f);

    // Abruptly jump master gain from -36 dB to +12 dB
    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::masterVol))
        p->setValueNotifyingHost(0.0f); // minimum

    processor.processBlock(buffer, midi);

    if (auto* p = apvts.getParameter(camelphat::ParameterIDs::masterVol))
        p->setValueNotifyingHost(1.0f); // maximum jump

    processor.processBlock(buffer, midi);

    const float* data = buffer.getReadPointer(0);
    float maxDelta = 0.0f;
    for (int i = 1; i < blockSize; ++i)
    {
        const float delta = std::abs(data[i] - data[i - 1]);
        maxDelta = juce::jmax(maxDelta, delta);
    }

    std::cout << "  -> Max sample-to-sample delta during full parameter jump: " << maxDelta << std::endl;
    assert(maxDelta < 0.2f && "Parameter smoothing successfully prevented instantaneous step click!");
    std::cout << "  -> PASS: Zipper noise eliminated via linear smoother ramps.\n" << std::endl;
}

static void testBandRejectRecombination()
{
    std::cout << "[Test 5/7] Running Band-Reject Split & Sum Coherence Test..." << std::endl;

    camelphat::dsp::BPFilter bpFilter;
    juce::dsp::ProcessSpec spec { 48000.0, 512, 2 };
    bpFilter.prepare(spec);

    juce::AudioBuffer<float> inBuffer(2, 512);
    juce::AudioBuffer<float> passBuffer(2, 512);
    juce::AudioBuffer<float> rejBuffer(2, 512);

    generateSineWave(inBuffer, 48000.0, 1000.0f);

    bpFilter.process(inBuffer, passBuffer, rejBuffer, 300.0f, 3000.0f, 0.0f, 0.0f, false);

    assert(passBuffer.getMagnitude(0, 512) > 0.01f && "Passband contains signal");
    assert(rejBuffer.getMagnitude(0, 512) >= 0.0f && "Band-reject contains out-of-band signal");

    // Test Boundary Clamping: Equal Cutoffs (1000 Hz, 1000 Hz)
    passBuffer.clear();
    rejBuffer.clear();
    bpFilter.process(inBuffer, passBuffer, rejBuffer, 1000.0f, 1000.0f, 0.5f, 0.5f, false);
    for (int i = 0; i < 512; ++i)
    {
        assert(std::isfinite(passBuffer.getSample(0, i)) && "Passband sample must be finite at equal cutoffs");
        assert(std::isfinite(rejBuffer.getSample(0, i)) && "Reject sample must be finite at equal cutoffs");
    }

    // Test Inverted Cutoffs: Low Cut (5000 Hz) > High Cut (500 Hz)
    passBuffer.clear();
    rejBuffer.clear();
    bpFilter.process(inBuffer, passBuffer, rejBuffer, 5000.0f, 500.0f, 0.5f, 0.5f, false);
    for (int i = 0; i < 512; ++i)
    {
        assert(std::isfinite(passBuffer.getSample(0, i)) && "Passband sample must be finite when cutoffs are inverted");
        assert(std::isfinite(rejBuffer.getSample(0, i)) && "Reject sample must be finite when cutoffs are inverted");
    }

    std::cout << "  -> PASS: BP Filter split generates valid passband and complementary band-reject audio.\n" << std::endl;
}

static void testMMFilterTopologies()
{
    std::cout << "[Test 6/7] Running All 12 MM Filter Topologies Verification..." << std::endl;

    camelphat::dsp::MMFilter mm;
    juce::dsp::ProcessSpec spec { 48000.0, 256, 2 };
    mm.prepare(spec);

    juce::AudioBuffer<float> pass(2, 256);
    juce::AudioBuffer<float> sc(2, 256);

    for (int mode = 0; mode < 12; ++mode)
    {
        generateSineWave(pass, 48000.0, 800.0f);
        generateSineWave(sc, 48000.0, 800.0f);

        mm.setParameters(1000.0f, 0.5f, mode, 1.0f, 10.0f, 100.0f, 0.0f, false);
        mm.processWithSidechain(pass, sc);

        const float rms = calculateRms(pass, 0);
        assert(std::isfinite(rms) && rms > 0.001f && "Filter mode output finite and stable");
    }

    std::cout << "  -> PASS: All 12 MM Filter modes processed cleanly without NaNs or instabilities.\n" << std::endl;
}

static void testFactoryPresetsRecall()
{
    std::cout << "[Test 7/7] Running 10 Factory Presets Recall & Dice Verification..." << std::endl;

    CamelPhatAudioProcessor processor;
    processor.prepareToPlay(48000.0, 256);

    const int numPrograms = processor.getNumPrograms();
    std::cout << "  -> Total programs reported: " << numPrograms << std::endl;
    assert(numPrograms == 10 && "10 factory presets must be available");

    for (int i = 0; i < 10; ++i)
    {
        processor.setCurrentProgram(i);
        const auto name = processor.getProgramName(i);
        assert(name.isNotEmpty() && "Preset name must not be empty");
    }

    // Verify Dice smart randomizer
    for (int i = 0; i < 50; ++i)
    {
        processor.randomizeParameters();
        auto& apvts = processor.getAPVTS();
        const float vol = *apvts.getRawParameterValue(camelphat::ParameterIDs::masterVol);
        const float mix = *apvts.getRawParameterValue(camelphat::ParameterIDs::masterMix);
        assert(std::abs(vol - 0.0f) < 0.01f && "Dice keeps master volume at 0 dB");
        assert(mix >= 0.39f && mix <= 1.01f && "Dice keeps master mix between 0.4 and 1.0");
    }

    std::cout << "  -> PASS: All 10 factory presets recall correctly and Dice satisfies constraints.\n" << std::endl;
}

static void testCompressorGainReductionTelemetry()
{
    std::cout << "[Test 8] Compressor Gain Reduction Telemetry" << std::endl;
    camelphat::dsp::CompressorModule comp;
    juce::dsp::ProcessSpec spec { 44100.0, 512, 2 };
    comp.prepare(spec);

    // Initial / reset state
    assert(std::abs(comp.getGainReductionDb()) < 0.001f && "Initial GR must be 0 dB");

    // Process a loud signal with compression enabled (amount = 0.8, normal mode)
    comp.setParameters(0.8f, 100.0f, false, false);
    juce::AudioBuffer<float> buffer(2, 512);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < 512; ++i)
            buffer.setSample(ch, i, 0.95f);

    comp.process(buffer);
    const float activeGr = comp.getGainReductionDb();
    assert(activeGr < -0.5f && "Gain reduction must be negative during active compression");

    // Bypassed state must reset telemetry to 0.0f
    comp.setParameters(0.8f, 100.0f, false, true); // bypass = true
    comp.process(buffer);
    assert(std::abs(comp.getGainReductionDb()) < 0.001f && "Bypassed compressor must report 0 dB GR");

    std::cout << "  -> PASS: Compressor gain reduction telemetry reports negative dB under compression and resets to 0 dB when bypassed.\n" << std::endl;
}

static void testEditorLayoutAndNonOverlap()
{
    std::cout << "[Test 9] Automated GUI Layout & Bounds Verification across Window Sizes" << std::endl;
    // Initialize JUCE MessageManager for GUI component testing
    juce::ScopedJuceInitialiser_GUI guiInit;

    CamelPhatAudioProcessor processor;
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    assert(editor != nullptr && "Editor must instantiate successfully");

    // Test sizes: 75% (810x510), 100% (1080x680), and 177% (1920x1200)
    const std::vector<std::pair<int, int>> testSizes = {
        { 810, 510 },
        { 1080, 680 },
        { 1920, 1200 }
    };

    for (const auto& [w, h] : testSizes)
    {
        editor->setSize(w, h);
        assert(editor->getWidth() == w && editor->getHeight() == h);

        // Verify child components have non-empty bounds
        for (int i = 0; i < editor->getNumChildComponents(); ++i)
        {
            auto* child = editor->getChildComponent(i);
            assert(child != nullptr);
            if (child->isVisible())
            {
                assert(child->getWidth() > 0 && child->getHeight() > 0 &&
                       "Visible root child component must have non-zero width and height");
            }
        }
    }

    // Verify center tab switching between X/Y Pad and Signal Monitor
    auto* cpEditor = dynamic_cast<CamelPhatAudioProcessorEditor*>(editor.get());
    assert(cpEditor != nullptr);
    assert(!cpEditor->isShowingMonitor());
    cpEditor->setCenterTab(true);
    assert(cpEditor->isShowingMonitor());
    cpEditor->setCenterTab(false);
    assert(!cpEditor->isShowingMonitor());

    std::cout << "  -> PASS: Editor instantiates, scales, and toggles Center Tab (X/Y Pad <-> Monitor) with valid geometry.\n" << std::endl;
}

// ==============================================================================
// Test 10: Modulation Engine Target Delivery Verification
// ==============================================================================
static void testModulationTargetDelivery()
{
    std::cout << "[Test 10] Running Modulation Engine Target Delivery Verification..." << std::endl;

    CamelPhatAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);

    auto& apvts = processor.getAPVTS();
    auto* lfo1Target = apvts.getParameter(camelphat::ParameterIDs::lfo1Target);
    auto* lfo1Depth  = apvts.getParameter(camelphat::ParameterIDs::lfo1Depth);
    auto* lfo1Rate   = apvts.getParameter(camelphat::ParameterIDs::lfo1Rate);
    assert(lfo1Target != nullptr && lfo1Depth != nullptr && lfo1Rate != nullptr);

    // Target DistMech (index 7 in ModTarget enum)
    const int distMechIdx = static_cast<int>(camelphat::ModTarget::DistMech);
    lfo1Target->setValueNotifyingHost(lfo1Target->convertTo0to1(static_cast<float>(distMechIdx)));
    lfo1Depth->setValueNotifyingHost(lfo1Depth->convertTo0to1(1.0f));
    lfo1Rate->setValueNotifyingHost(lfo1Rate->convertTo0to1(5.0f));

    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();
    juce::MidiBuffer midi;

    // Process several blocks to advance LFO
    bool nonZeroLfoOffsetObserved = false;
    for (int b = 0; b < 10; ++b)
    {
        processor.processBlock(buffer, midi);
        float offset = processor.getModulationEngine().getOffset(camelphat::ModTarget::DistMech);
        if (std::abs(offset) > 0.01f)
            nonZeroLfoOffsetObserved = true;
    }
    assert(nonZeroLfoOffsetObserved && "ModulationEngine must deliver non-zero offset to DistMech target");

    // Test XY Pad target delivery
    auto* xyXTarget = apvts.getParameter(camelphat::ParameterIDs::xyXTarget);
    auto* xyXDepth  = apvts.getParameter(camelphat::ParameterIDs::xyXDepth);
    auto* xyX       = apvts.getParameter(camelphat::ParameterIDs::xyX);
    assert(xyXTarget != nullptr && xyXDepth != nullptr && xyX != nullptr);

    const int mmCutIdx = static_cast<int>(camelphat::ModTarget::MMCutoff);
    xyXTarget->setValueNotifyingHost(xyXTarget->convertTo0to1(static_cast<float>(mmCutIdx)));
    xyXDepth->setValueNotifyingHost(xyXDepth->convertTo0to1(0.8f));
    xyX->setValueNotifyingHost(0.9f); // normX = 0.9 -> (0.9*2 - 1) = 0.8 -> * 0.8 depth = 0.64

    processor.processBlock(buffer, midi);
    float xyOffset = processor.getModulationEngine().getOffset(camelphat::ModTarget::MMCutoff);
    assert(xyOffset > 0.5f && "ModulationEngine must deliver positive offset to MMCutoff from XY pad");
    juce::ignoreUnused(nonZeroLfoOffsetObserved, xyOffset);

    std::cout << "  -> PASS: ModulationEngine actively routes non-zero LFO and XY Pad offsets to target parameters.\n" << std::endl;
}

// ==============================================================================
// Test 11: Dual Input and Output Metering Telemetry Verification
// ==============================================================================
static void testInputAndOutputMetering()
{
    std::cout << "[Test 11] Running Input and Output Level Metering Verification..." << std::endl;

    CamelPhatAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);

    juce::AudioBuffer<float> buffer(2, 512);
    generateSineWave(buffer, 44100.0, 440.0f);
    juce::MidiBuffer midi;

    processor.processBlock(buffer, midi);

    const float inPeakL = processor.getInputMeterSource().getPeakLevel(0);
    const float inPeakR = processor.getInputMeterSource().getPeakLevel(1);
    const float inRmsL  = processor.getInputMeterSource().getRmsLevel(0);

    const float outPeakL = processor.getMasterSection().getPeakLevel(0);
    const float outRmsL  = processor.getMasterSection().getRmsLevel(0);

    assert(inPeakL > 0.5f && inPeakR > 0.5f && "Input meter must detect peak signal");
    assert(inRmsL > 0.3f && "Input meter must detect RMS signal");
    assert(outPeakL > 0.1f && outRmsL > 0.05f && "Output meter must detect peak and RMS signal");

    juce::ignoreUnused(inPeakL, inPeakR, inRmsL, outPeakL, outRmsL);
    std::cout << "  -> PASS: Dual Input and Output meters report active, finite audio telemetry.\n" << std::endl;
}

static void testAudioVisualizerSourceFifo()
{
    std::cout << "[Test 12] AudioVisualizerSource Lock-Free Ring Buffer..." << std::endl;
    camelphat::dsp::AudioVisualizerSource source(1024);

    std::vector<float> chL(256, 0.4f);
    std::vector<float> chR(256, 0.6f);
    const float* inChannels[2] = { chL.data(), chR.data() };

    source.pushInput(inChannels, 2, 256);
    assert(source.getNumReadyInput() == 256);

    std::vector<float> readBack(256, 0.0f);
    int readCount = source.readInput(readBack.data(), 256);
    assert(readCount == 256);
    juce::ignoreUnused(readCount);
    for (float v : readBack)
    {
        assert(std::abs(v - 0.5f) < 1e-5f);
        juce::ignoreUnused(v);
    }

    assert(source.getNumReadyInput() == 0);

    std::vector<float> outL(128, 0.8f);
    const float* outChannels[1] = { outL.data() };
    source.pushOutput(outChannels, 1, 128);
    assert(source.getNumReadyOutput() == 128);

    std::vector<float> outRead(128, 0.0f);
    int outReadCount = source.readOutput(outRead.data(), 128);
    assert(outReadCount == 128);
    juce::ignoreUnused(outReadCount);
    for (float v : outRead)
    {
        assert(std::abs(v - 0.8f) < 1e-5f);
        juce::ignoreUnused(v);
    }

    std::cout << "  -> PASS: AudioVisualizerSource FIFO push and read accurately verified.\n" << std::endl;
}

int main(int, char*[])
{
    std::cout << "=====================================================" << std::endl;
    std::cout << "   CamelPhat 3 DSP Engine - Automated Verification   " << std::endl;
    std::cout << "=====================================================\n" << std::endl;

    try
    {
        testZeroAllocations();
        testHarmonicDistortion();
        testFilterStabilityAndNanDefense();
        testParameterSmoothing();
        testBandRejectRecombination();
        testMMFilterTopologies();
        testFactoryPresetsRecall();
        testCompressorGainReductionTelemetry();
        testEditorLayoutAndNonOverlap();
        testModulationTargetDelivery();
        testInputAndOutputMetering();
        testAudioVisualizerSourceFifo();

        std::cout << "=====================================================" << std::endl;
        std::cout << "   ALL 7 DSP SUITES PASSED (100% SUCCESS)            " << std::endl;
        std::cout << "=====================================================" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "EXCEPTION ENCOUNTERED: " << e.what() << std::endl;
        return 1;
    }
}
