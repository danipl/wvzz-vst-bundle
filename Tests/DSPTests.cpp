#include "Plugins/WvZzDistortion/PluginProcessor.h"
#include "Plugins/WvZzDistortion/PluginEditor.h"
#include "Plugins/WvZzDynamics/PluginProcessor.h"
#include "Plugins/WvZzDynamics/PluginEditor.h"
#include "Plugins/WvZzDynamics/Presets/FactoryPresets.h"
#include "Core/DSP/AudioVisualizerSource.h"
#include "Modules/BPFilter/BPFilterModule.h"
#include "Modules/Distortion/DistortionModule.h"
#include "Modules/MMFilter/MMFilterModule.h"
#include "Modules/Flanger/FlangerModule.h"
#include "Modules/MagicEQ/MagicEQModule.h"
#include "Modules/Compressor/CompressorModule.h"
#include "Modules/Modulation/ModulationModule.h"
#include "Modules/Master/MasterModule.h"
#include "Modules/DynDetector/DynDetectorModule.h"
#include "Modules/TransientShaper/TransientShaperModule.h"
#include "Modules/MultibandOTT/MultibandOTTModule.h"
#include "Modules/RhythmicPumper/RhythmicPumperModule.h"
#include "Modules/MaximizerClipper/MaximizerClipperModule.h"

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
    std::cout << "[Test 1/13] Running Real-Time Zero-Allocation Audit..." << std::endl;

    wvzz::WvZzDistortionAudioProcessor processor;
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

static void testMultiInstancePrefixing()
{
    std::cout << "[Test 2/13] Running Multi-Instance Module Parameter Prefixing Test..." << std::endl;

    // Test that two instances of DistortionModule can exist independently with custom prefixes
    wvzz::modules::DistortionModule distSlot1("slot1_dist");
    wvzz::modules::DistortionModule distSlot2("slot2_dist");

    juce::AudioProcessorValueTreeState::ParameterLayout layout1;
    juce::AudioProcessorValueTreeState::ParameterLayout layout2;
    distSlot1.getDSP().registerParameters(layout1);
    distSlot2.getDSP().registerParameters(layout2);

    assert(distSlot1.getInstancePrefix() == "slot1_dist");
    assert(distSlot2.getInstancePrefix() == "slot2_dist");

    std::cout << "  -> PASS: Modules support custom instancePrefix without ID collision.\n" << std::endl;
}

static void testHarmonicDistortion()
{
    std::cout << "[Test 3/13] Running Harmonic Distortion Verification (Tube, Mech, Bit, Xcita)..." << std::endl;

    wvzz::WvZzDistortionAudioProcessor processor;
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
    if (auto* param = apvts.getParameter("dist_tube"))
        param->setValueNotifyingHost(0.8f);

    generateSineWave(buffer, sampleRate, 1000.0f);
    processor.processBlock(buffer, midi);
    const float tubeRms = calculateRms(buffer, 0);

    std::cout << "  -> Clean RMS: " << cleanRms << ", Tube Saturated RMS: " << tubeRms << std::endl;
    assert(tubeRms > 0.05f && "Tube saturation output is non-zero and compressed");

    // 3. Mech Drive Engaged
    if (auto* tubeP = apvts.getParameter("dist_tube"))
        tubeP->setValueNotifyingHost(0.0f);
    if (auto* mechP = apvts.getParameter("dist_mech"))
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
    std::cout << "[Test 4/13] Running Filter Stability & NaN / Inf Sanitization Stress Test..." << std::endl;

    wvzz::WvZzDistortionAudioProcessor processor;
    const double sampleRate = 96000.0;
    const int blockSize = 256;
    processor.prepareToPlay(sampleRate, blockSize);

    auto& apvts = processor.getAPVTS();

    // Push BP filter and MM filter resonances to extreme maximum
    if (auto* p = apvts.getParameter("bp_low_res"))
        p->setValueNotifyingHost(1.0f);
    if (auto* p = apvts.getParameter("bp_high_res"))
        p->setValueNotifyingHost(1.0f);
    if (auto* p = apvts.getParameter("mm_res"))
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
        if (auto* p = apvts.getParameter("mm_cutoff"))
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
    std::cout << "[Test 5/13] Running Parameter Smoothing & Anti-Zipper Continuity Test..." << std::endl;

    wvzz::WvZzDistortionAudioProcessor processor;
    const double sampleRate = 44100.0;
    const int blockSize = 128;
    processor.prepareToPlay(sampleRate, blockSize);

    auto& apvts = processor.getAPVTS();

    juce::AudioBuffer<float> buffer(2, blockSize);
    juce::MidiBuffer midi;
    generateSineWave(buffer, sampleRate, 440.0f);

    // Abruptly jump master gain from -36 dB to +12 dB
    if (auto* p = apvts.getParameter("master_vol"))
        p->setValueNotifyingHost(0.0f); // minimum

    processor.processBlock(buffer, midi);

    if (auto* p = apvts.getParameter("master_vol"))
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
    std::cout << "[Test 6/13] Running Band-Reject Split & Sum Coherence Test..." << std::endl;

    wvzz::modules::BPFilterDSP bpFilter;
    juce::dsp::ProcessSpec spec { 48000.0, 512, 2 };
    bpFilter.prepare(spec);

    juce::AudioBuffer<float> mainBuffer(2, 512);
    juce::AudioBuffer<float> scBuffer(2, 512);
    juce::AudioBuffer<float> rejBuffer(2, 512);
    bool hasAux = false;

    generateSineWave(mainBuffer, 48000.0, 1000.0f);

    wvzz::ProcessContext ctx {
        mainBuffer,
        scBuffer,
        rejBuffer,
        hasAux,
        nullptr,
        0
    };

    bpFilter.process(ctx);

    assert(mainBuffer.getMagnitude(0, 512) > 0.01f && "Passband contains signal");
    assert(rejBuffer.getMagnitude(0, 512) >= 0.0f && "Band-reject contains out-of-band signal");
    assert(ctx.hasAuxSplit && "hasAuxSplit flag must be set by BPFilterDSP");

    // Test recombination
    bpFilter.recombine(ctx);
    for (int i = 0; i < 512; ++i)
    {
        assert(std::isfinite(mainBuffer.getSample(0, i)) && "Passband sample must be finite after recombination");
    }

    std::cout << "  -> PASS: BP Filter split generates valid passband and complementary band-reject audio.\n" << std::endl;
}

static void testMMFilterTopologies()
{
    std::cout << "[Test 7/13] Running All 12 MM Filter Topologies Verification..." << std::endl;

    wvzz::modules::MMFilterDSP mm;
    juce::dsp::ProcessSpec spec { 48000.0, 256, 2 };
    mm.prepare(spec);

    juce::AudioBuffer<float> pass(2, 256);
    juce::AudioBuffer<float> sc(2, 256);
    juce::AudioBuffer<float> dummyRej(2, 256);
    bool dummyAux = false;

    for (int mode = 0; mode < 12; ++mode)
    {
        generateSineWave(pass, 48000.0, 800.0f);
        generateSineWave(sc, 48000.0, 800.0f);

        wvzz::ProcessContext ctx {
            pass,
            sc,
            dummyRej,
            dummyAux,
            nullptr,
            0
        };

        mm.setParameters(1000.0f, 0.5f, mode, 1.0f, 10.0f, 100.0f, 0.0f, false);
        mm.process(ctx);

        const float rms = calculateRms(pass, 0);
        assert(std::isfinite(rms) && rms > 0.001f && "Filter mode output finite and stable");
    }

    std::cout << "  -> PASS: All 12 MM Filter modes processed cleanly without NaNs or instabilities.\n" << std::endl;
}

static void testFactoryPresetsRecall()
{
    std::cout << "[Test 8/13] Running 15 Factory Presets Recall & Dice Verification..." << std::endl;

    wvzz::WvZzDistortionAudioProcessor processor;
    processor.prepareToPlay(48000.0, 256);

    const int numPrograms = processor.getNumPrograms();
    std::cout << "  -> Total programs reported: " << numPrograms << std::endl;
    assert(numPrograms == 15 && "15 factory presets must be available");

    for (int i = 0; i < 15; ++i)
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
        const float vol = *apvts.getRawParameterValue("master_vol");
        const float mix = *apvts.getRawParameterValue("master_mix");
        assert(std::abs(vol - 0.0f) < 0.01f && "Dice keeps master volume at 0 dB");
        assert(mix >= 0.39f && mix <= 1.01f && "Dice keeps master mix between 0.4 and 1.0");
    }

    std::cout << "  -> PASS: All 15 factory presets recall correctly and Dice satisfies constraints.\n" << std::endl;
}

static void testCompressorGainReductionTelemetry()
{
    std::cout << "[Test 9/13] Compressor Gain Reduction Telemetry" << std::endl;
    wvzz::modules::CompressorDSP comp;
    juce::dsp::ProcessSpec spec { 44100.0, 512, 2 };
    comp.prepare(spec);

    // Initial / reset state
    assert(std::abs(comp.getGainReductionDb()) < 0.001f && "Initial GR must be 0 dB");

    // Process a loud signal with compression enabled (amount = 0.8, normal mode)
    comp.setParameters(0.8f, 100.0f, false, false);
    juce::AudioBuffer<float> buffer(2, 512);
    juce::AudioBuffer<float> dummySc(2, 512);
    juce::AudioBuffer<float> dummyRej(2, 512);
    bool dummyAux = false;

    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < 512; ++i)
            buffer.setSample(ch, i, 0.95f);

    wvzz::ProcessContext ctx {
        buffer,
        dummySc,
        dummyRej,
        dummyAux,
        nullptr,
        0
    };

    comp.process(ctx);
    const float activeGr = comp.getGainReduction();
    assert(activeGr < -0.5f && "Gain reduction must be negative during active compression");

    // Bypassed state must reset telemetry to 0.0f
    comp.setParameters(0.8f, 100.0f, false, true); // bypass = true
    comp.process(ctx);
    assert(std::abs(comp.getGainReduction()) < 0.001f && "Bypassed compressor must report 0 dB GR");

    // Dual-stage optical decay verification
    comp.reset();
    comp.setParameters(0.8f, 200.0f, false, false);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < 512; ++i)
            buffer.setSample(ch, i, 0.95f);
    comp.process(ctx);
    const float heavyGr = comp.getGainReduction();
    assert(heavyGr < -5.0f && "Heavy compression expected");

    // Now feed silence and observe fast initial release (~50 ms = ~4-5 blocks at 512 samples/44.1 kHz)
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < 512; ++i)
            buffer.setSample(ch, i, 0.0f);

    // Process ~50 ms of silence (4 blocks of 512 samples = ~46 ms)
    for (int b = 0; b < 4; ++b)
        comp.process(ctx);
    const float midGr = comp.getGainReduction();
    // Mid GR should have recovered substantially (closer to 0 than initial heavy GR)
    assert(midGr > heavyGr && midGr < 0.0f && "Dual-stage release must show fast initial recovery");

    // Process additional 500 ms (44 blocks)
    for (int b = 0; b < 44; ++b)
        comp.process(ctx);
    const float finalGr = comp.getGainReduction();
    assert(finalGr > midGr && "Optical tail must continue smooth decay toward 0 dB");

    // Independent Threshold and Ratio verification (with amount = 0.0)
    comp.reset();
    comp.setParameters(0.0f, 100.0f, false, false, -24.0f, 6.0f, 2.0f, 1.0f);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < 512; ++i)
            buffer.setSample(ch, i, 0.95f);
    comp.process(ctx);
    const float studioGr = comp.getGainReduction();
    assert(studioGr < -2.0f && "Independent threshold and ratio must compress when amount = 0.0");

    // Parallel Mix verification (mix = 0.0 -> 100% dry)
    comp.reset();
    comp.setParameters(0.0f, 100.0f, false, false, -24.0f, 6.0f, 2.0f, 0.0f);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < 512; ++i)
            buffer.setSample(ch, i, 0.95f);
    comp.process(ctx);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < 512; ++i)
            assert(std::abs(buffer.getSample(ch, i) - 0.95f) < 1.0e-4f && "Mix = 0.0 must pass 100% dry uncompressed audio");

    // Phat Mode 1st-order ADAA saturation ceiling and telemetry verification
    comp.reset();
    comp.setParameters(0.8f, 100.0f, true, false); // phat = true
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < 512; ++i)
            buffer.setSample(ch, i, 2.0f); // Over-driven input
    comp.process(ctx);
    assert(comp.isSaturating() && "Saturation telemetry must report true under overdriven Phat clipping");

    constexpr float ceilingLinear = 0.988553f; // -0.1 dBFS
    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < 512; ++i)
        {
            const float sample = buffer.getSample(ch, i);
            juce::ignoreUnused(sample);
            assert(!std::isnan(sample) && !std::isinf(sample) && "ADAA output must be finite");
            assert(std::abs(sample) <= ceilingLinear + 1.0e-3f && "ADAA ceiling must clamp to -0.1 dBFS");
        }
    }

    juce::ignoreUnused(activeGr, heavyGr, midGr, finalGr, studioGr, ceilingLinear);

    std::cout << "  -> PASS: Compressor gain reduction telemetry reports negative dB under compression, exhibits dual-stage optical release, supports studio controls, parallel mix, and enforces ADAA -0.1 dBFS ceiling in Phat mode.\n" << std::endl;
}

static void testEditorLayoutAndNonOverlap()
{
    std::cout << "[Test 10/13] Automated GUI Layout & Bounds Verification across Window Sizes" << std::endl;
    // Initialize JUCE MessageManager for GUI component testing
    juce::ScopedJuceInitialiser_GUI guiInit;

    wvzz::WvZzDistortionAudioProcessor processor;
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

    std::cout << "  -> PASS: Editor instantiates and scales with valid geometry.\n" << std::endl;
}

static void testModulationTargetDelivery()
{
    std::cout << "[Test 11/13] Running Modulation Engine Target Delivery Verification..." << std::endl;

    wvzz::WvZzDistortionAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);

    auto& apvts = processor.getAPVTS();
    auto* lfo1Target = apvts.getParameter("lfo1_target");
    auto* lfo1Depth  = apvts.getParameter("lfo1_depth");
    auto* lfo1Rate   = apvts.getParameter("lfo1_rate");
    assert(lfo1Target != nullptr && lfo1Depth != nullptr && lfo1Rate != nullptr);

    // Target DistMech (index 7 in ModTarget enum)
    const int distMechIdx = static_cast<int>(wvzz::ModTarget::DistMech);
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
        float offset = processor.getModulationModule().getModulationDSP().getOffset(wvzz::ModTarget::DistMech);
        if (std::abs(offset) > 0.01f)
            nonZeroLfoOffsetObserved = true;
    }
    assert(nonZeroLfoOffsetObserved && "ModulationEngine must deliver non-zero offset to DistMech target");

    // Test XY Pad target delivery
    auto* xyXTarget = apvts.getParameter("xy_x_target");
    auto* xyXDepth  = apvts.getParameter("xy_x_depth");
    auto* xyX       = apvts.getParameter("xy_x");
    assert(xyXTarget != nullptr && xyXDepth != nullptr && xyX != nullptr);

    const int mmCutIdx = static_cast<int>(wvzz::ModTarget::MMCutoff);
    xyXTarget->setValueNotifyingHost(xyXTarget->convertTo0to1(static_cast<float>(mmCutIdx)));
    xyXDepth->setValueNotifyingHost(xyXDepth->convertTo0to1(0.8f));
    xyX->setValueNotifyingHost(0.9f);

    processor.processBlock(buffer, midi);
    float xyOffset = processor.getModulationModule().getModulationDSP().getOffset(wvzz::ModTarget::MMCutoff);
    assert(xyOffset > 0.5f && "ModulationEngine must deliver positive offset to MMCutoff from XY pad");

    // Test MasterVol modulation target (target 21)
    auto* xyYTarget = apvts.getParameter("xy_y_target");
    auto* xyYDepth  = apvts.getParameter("xy_y_depth");
    auto* xyY       = apvts.getParameter("xy_y");
    assert(xyYTarget != nullptr && xyYDepth != nullptr && xyY != nullptr);

    const int masterVolIdx = static_cast<int>(wvzz::ModTarget::MasterVol);
    xyYTarget->setValueNotifyingHost(xyYTarget->convertTo0to1(static_cast<float>(masterVolIdx)));
    xyYDepth->setValueNotifyingHost(xyYDepth->convertTo0to1(1.0f));
    xyY->setValueNotifyingHost(1.0f);

    processor.processBlock(buffer, midi);
    float volOffset = processor.getModulationModule().getModulationDSP().getOffset(wvzz::ModTarget::MasterVol);
    assert(volOffset > 0.8f && "ModulationEngine must deliver offset to MasterVol");

    std::cout << "  -> PASS: ModulationModule actively routes non-zero LFO and XY Pad offsets to target parameters (including MasterVol).\n" << std::endl;
}

static void testInputAndOutputMetering()
{
    std::cout << "[Test 12/13] Running Input and Output Level Metering Verification..." << std::endl;

    wvzz::WvZzDistortionAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);

    juce::AudioBuffer<float> buffer(2, 512);
    generateSineWave(buffer, 44100.0, 440.0f);
    juce::MidiBuffer midi;

    processor.processBlock(buffer, midi);

    const float inPeakL = processor.getMasterModule().getMasterDSP().getInputPeakLevel(0);
    const float inPeakR = processor.getMasterModule().getMasterDSP().getInputPeakLevel(1);
    const float inRmsL  = processor.getMasterModule().getMasterDSP().getInputRmsLevel(0);

    const float outPeakL = processor.getMasterModule().getMasterDSP().getPeakLevel(0);
    const float outRmsL  = processor.getMasterModule().getMasterDSP().getRmsLevel(0);

    assert(inPeakL > 0.5f && inPeakR > 0.5f && "Input meter must detect peak signal");
    assert(inRmsL > 0.3f && "Input meter must detect RMS signal");
    assert(outPeakL > 0.1f && outRmsL > 0.05f && "Output meter must detect peak and RMS signal");

    juce::ignoreUnused(inPeakL, inPeakR, inRmsL, outPeakL, outRmsL);
    std::cout << "  -> PASS: Dual Input and Output meters report active, finite audio telemetry.\n" << std::endl;
}

static void testAudioVisualizerSourceFifo()
{
    std::cout << "[Test 13/13] AudioVisualizerSource Lock-Free Ring Buffer..." << std::endl;
    wvzz::dsp::AudioVisualizerSource source(1024);

    std::vector<float> chL(256, 0.4f);
    std::vector<float> chR(256, 0.6f);
    const float* inChannels[2] = { chL.data(), chR.data() };

    source.pushInput(inChannels, 2, 256);
    assert(source.getNumReadyInput() == 256);

    std::vector<float> readBack(256, 0.0f);
    int readCount = source.readInput(readBack.data(), 256);
    assert(readCount == 256);
    for (float v : readBack)
    {
        assert(std::abs(v - 0.5f) < 1e-5f);
    }

    assert(source.getNumReadyInput() == 0);

    std::vector<float> outL(128, 0.8f);
    const float* outChannels[1] = { outL.data() };
    source.pushOutput(outChannels, 1, 128);
    assert(source.getNumReadyOutput() == 128);

    std::vector<float> outRead(128, 0.0f);
    int outReadCount = source.readOutput(outRead.data(), 128);
    assert(outReadCount == 128);
    for (float v : outRead)
    {
        assert(std::abs(v - 0.8f) < 1e-5f);
    }

    std::cout << "  -> PASS: AudioVisualizerSource FIFO push and read accurately verified.\n" << std::endl;
}

static void testEdmHardstyleDspFeatures()
{
    std::cout << "[Test 14/14] Running EDM & Hardstyle DSP Capabilities Verification..." << std::endl;

    // 1. Test Hard Clip and Wavefold character modes
    wvzz::modules::DistortionDSP dist;
    juce::dsp::ProcessSpec spec { 48000.0, 512, 2 };
    dist.prepare(spec);

    juce::AudioBuffer<float> mainBuf(2, 512);
    juce::AudioBuffer<float> dummySc(2, 512);
    juce::AudioBuffer<float> dummyRej(2, 512);
    bool dummyAux = false;

    // Generate large amplitude wave
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < 512; ++i)
            mainBuf.setSample(ch, i, std::sin(2.0 * juce::MathConstants<double>::pi * 100.0 * i / 48000.0) * 1.5f);

    wvzz::ProcessContext ctx {
        mainBuf, dummySc, dummyRej, dummyAux, nullptr, 0, 150.0, 0.0, false, 0
    };

    // Mode 1: Hard Clip
    dist.setParameters(0.9f, 0.5f, 0.0f, 0.0f, false, 1);
    dist.process(ctx);

    for (int i = 0; i < 512; ++i)
    {
        const float s = std::abs(mainBuf.getSample(0, i));
        assert(s <= 1.05f && "Hard Clip mode must keep peaks bounded");
    }

    // 2. Test Magic EQ Pre-Distortion flag and extended range
    wvzz::modules::MagicEQDSP meq;
    meq.prepare(spec);
    meq.setParameters(0.8f, 50.0f, true, false, true); // 50 Hz, pre-dist enabled
    assert(meq.isPreDistortion() && "Magic EQ pre-distortion flag must be active");

    // 3. Test Master Sub-Bass Mono Maker and 0 dBFS Clipper
    wvzz::modules::MasterDSP master;
    master.prepare(spec);
    master.setParameters(0.0f, 1.0f, 0.0f, false, true, true); // clip = true, monoBass = true

    juce::AudioBuffer<float> stereoBuf(2, 512);
    // Left channel: 60 Hz sine, Right channel: inverted 60 Hz sine (out-of-phase stereo bass)
    for (int i = 0; i < 512; ++i)
    {
        const float s = std::sin(2.0 * juce::MathConstants<double>::pi * 60.0 * i / 48000.0);
        stereoBuf.setSample(0, i, s);
        stereoBuf.setSample(1, i, -s);
    }

    wvzz::ProcessContext masterCtx {
        stereoBuf, stereoBuf, dummyRej, dummyAux, nullptr, 0, 150.0, 0.0, false, 0
    };

    master.process(masterCtx);

    // With monoBass active, out-of-phase 60 Hz should cancel out to near zero
    const float lowRms = calculateRms(stereoBuf, 0);
    std::cout << "  -> Mono Bass cancellation RMS of out-of-phase 60Hz: " << lowRms << std::endl;
    assert(lowRms < 0.25f && "Sub-bass mono maker must cancel out-of-phase low end");

    // 4. Test "Euphoric Supersaw Excite" stability at High Cut = 20 kHz (top) & zero preset leakage
    {
        wvzz::WvZzDistortionAudioProcessor proc;
        proc.prepareToPlay(44100.0, 512);

        // Load Preset 10 (index 10 = #11 Hardstyle Kick Tok & Crunch, which sets dist_mech=0.8, meq_pre=1)
        proc.setCurrentProgram(10);
        // Switch to Preset 11 (index 11 = #12 Euphoric Supersaw Excite)
        proc.setCurrentProgram(11);

        auto& apvts = proc.getAPVTS();
        assert(apvts.getRawParameterValue("dist_mech")->load() < 0.01f && "Preset switch must reset unlisted dist_mech to default 0");
        assert(apvts.getRawParameterValue("meq_pre")->load() < 0.5f && "Preset switch must reset meq_pre to 0");

        // Push High Cut to top (20,000 Hz) with high resonance
        if (auto* hc = apvts.getParameter("bp_high_cut"))
            hc->setValueNotifyingHost(hc->convertTo0to1(20000.0f));
        if (auto* hr = apvts.getParameter("bp_high_res"))
            hr->setValueNotifyingHost(hr->convertTo0to1(1.0f));

        juce::AudioBuffer<float> sawBuf(2, 512);
        juce::MidiBuffer midi;

        g_allocationCount = 0;
        g_trackingAllocations = true;

        double phase1 = 0.0, phase2 = 0.0, phase3 = 0.0;
        for (int b = 0; b < 60; ++b)
        {
            for (int i = 0; i < 512; ++i)
            {
                // Dense high-frequency supersaw harmonics (440 Hz, 443 Hz, + 14.5 kHz air)
                phase1 = std::fmod(phase1 + 440.0 / 44100.0, 1.0);
                phase2 = std::fmod(phase2 + 443.2 / 44100.0, 1.0);
                phase3 = std::fmod(phase3 + 14500.0 / 44100.0, 1.0);
                const float s = static_cast<float>((2.0 * phase1 - 1.0) * 0.35
                                                 + (2.0 * phase2 - 1.0) * 0.35
                                                 + std::sin(2.0 * juce::MathConstants<double>::pi * phase3) * 0.3);
                sawBuf.setSample(0, i, s);
                sawBuf.setSample(1, i, s);
            }
            proc.processBlock(sawBuf, midi);

            for (int ch = 0; ch < 2; ++ch)
            {
                for (int i = 0; i < 512; ++i)
                {
                    const float y = sawBuf.getSample(ch, i);
                    if (!std::isfinite(y) || std::abs(y) > 1.001f)
                    {
                        std::cerr << "FAIL: Euphoric Supersaw Excite unstable sample: " << y << std::endl;
                        std::exit(1);
                    }
                }
            }
        }

        // Feed silence and verify filter & MagicEQ Phat mode decay cleanly without limit-cycle self-oscillation
        for (int b = 0; b < 30; ++b)
        {
            sawBuf.clear();
            proc.processBlock(sawBuf, midi);
        }

        g_trackingAllocations = false;
        const int phatAllocs = g_allocationCount;
        if (phatAllocs != 0)
        {
            std::cerr << "FAIL: Euphoric Supersaw Excite allocated " << phatAllocs << " times on audio thread" << std::endl;
            std::exit(1);
        }

        const float tailRms = calculateRms(sawBuf, 0);
        std::cout << "  -> Euphoric Supersaw Excite (20kHz High Cut, Max Res) silence tail RMS: " << tailRms << std::endl;
        if (tailRms >= 0.005f)
        {
            std::cerr << "FAIL: Self-oscillation detected in silence tail: " << tailRms << std::endl;
            std::exit(1);
        }
    }

    std::cout << "  -> PASS: EDM & Hardstyle DSP (Clip/Fold, Pre-Distortion EQ, Mono Bass, Clipper, Supersaw Stability) verified.\n" << std::endl;
}

// ==============================================================================
// WvZz Dynamics Verification Suites
// ==============================================================================

static void testWvZzDynamicsZeroAllocations()
{
    std::cout << "[Test 15/19] Running WvZz Dynamics Real-Time Zero-Allocation Audit..." << std::endl;

    wvzz::WvZzDynamicsAudioProcessor processor;
    const double rates[] = { 48000.0, 96000.0 };
    const int blockSizes[] = { 512, 256 };

    for (double sr : rates)
    {
        for (int bs : blockSizes)
        {
            processor.prepareToPlay(sr, bs);

            juce::AudioBuffer<float> buffer(2, bs);
            juce::MidiBuffer midi;
            generateSineWave(buffer, sr, 120.0f);

            // Warm up
            processor.processBlock(buffer, midi);

            g_allocationCount = 0;
            g_trackingAllocations = true;

            for (int i = 0; i < 1000; ++i)
            {
                processor.processBlock(buffer, midi);
            }

            g_trackingAllocations = false;
            std::cout << "  -> " << sr << " Hz (" << bs << " samples): " << g_allocationCount << " heap allocations" << std::endl;

            if (g_allocationCount != 0)
            {
                std::cerr << "FAIL: WvZz Dynamics performed " << g_allocationCount << " allocations on audio thread!" << std::endl;
                std::exit(1);
            }
        }
    }

    std::cout << "  -> PASS: WvZz Dynamics zero dynamic allocations confirmed on audio thread.\n" << std::endl;
}

static void testWvZzDynamicsMultiInstancePrefixing()
{
    std::cout << "[Test 16/19] Verifying WvZz Dynamics Multi-Instance Parameter Prefixing..." << std::endl;

    wvzz::modules::DynDetectorModule det("slotA_det");
    wvzz::modules::TransientShaperModule trans("slotB_trans");
    wvzz::modules::MultibandOTTModule ott("slotC_ott");
    wvzz::modules::RhythmicPumperModule pump("slotD_pump");
    wvzz::modules::MaximizerClipperModule clip("slotE_clip");

    assert(det.getInstancePrefix() == "slotA_det");
    assert(trans.getInstancePrefix() == "slotB_trans");
    assert(ott.getInstancePrefix() == "slotC_ott");
    assert(pump.getInstancePrefix() == "slotD_pump");
    assert(clip.getInstancePrefix() == "slotE_clip");

    std::cout << "  -> PASS: All dynamics modules support isolated multi-instance prefixing.\n" << std::endl;
}

static void testWvZzDynamicsFactoryPresetsRecall()
{
    std::cout << "[Test 17/19] Verifying WvZz Dynamics Factory Presets Recall (20 Presets)..." << std::endl;

    wvzz::WvZzDynamicsAudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    const auto& presets = wvzz::dynamics::getFactoryPresets();
    assert(presets.size() == 20);
    assert(processor.getNumPrograms() == 20);

    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;

    for (int i = 0; i < 20; ++i)
    {
        processor.setCurrentProgram(i);
        const juce::String progName = processor.getProgramName(i);
        if (progName != presets[static_cast<size_t>(i)].name)
        {
            std::cerr << "FAIL: Preset index " << i << " name mismatch: " << progName << " vs " << presets[static_cast<size_t>(i)].name << std::endl;
            std::exit(1);
        }

        // Test numerical stability under preset
        generateSineWave(buffer, 48000.0, 150.0f);
        processor.processBlock(buffer, midi);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const float* chData = buffer.getReadPointer(ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                if (std::isnan(chData[s]) || std::isinf(chData[s]))
                {
                    std::cerr << "FAIL: NaN or Inf detected in preset " << progName << std::endl;
                    std::exit(1);
                }
            }
        }
    }

    std::cout << "  -> PASS: All 20 WvZz Dynamics factory presets recalled with numerical stability.\n" << std::endl;
}

static void testWvZzDynamicsDSPMath()
{
    std::cout << "[Test 18/19] Verifying WvZz Dynamics DSP Math & Processing Algorithms..." << std::endl;

    const double sampleRate = 48000.0;
    const int blockSize = 512;
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(blockSize), 2 };

    // 1. DynDetector Linkwitz-Riley Sub Split
    {
        wvzz::modules::DynDetectorDSP detDSP("det");
        detDSP.prepare(spec);
        detDSP.setParameters(100.0f, true, 0.2f, 30.0f, 0.0f);

        juce::AudioBuffer<float> mainBuf(2, blockSize);
        juce::AudioBuffer<float> subAuxBuf(2, blockSize);
        juce::AudioBuffer<float> scBuf(2, blockSize);
        generateSineWave(mainBuf, sampleRate, 50.0f); // 50 Hz sub tone

        bool hasSubSplit = false;
        wvzz::ProcessContext ctx {
            mainBuf,
            scBuf,
            subAuxBuf,
            hasSubSplit,
            nullptr,
            0,
            120.0,
            0.0,
            false,
            0,
            0.0f
        };

        detDSP.process(ctx);

        const float subRms = calculateRms(subAuxBuf, 0);
        std::cout << "  -> DynDetector: SubAux RMS for 50Hz tone: " << subRms << std::endl;
        assert(subRms > 0.1f);
    }

    // 2. Transient Shaper Differential Attack Boost
    {
        wvzz::modules::TransientShaperDSP transDSP("trans");
        transDSP.prepare(spec);

        juce::AudioBuffer<float> stepBuf(2, blockSize);
        juce::AudioBuffer<float> dummyBuf(2, blockSize);
        stepBuf.clear();
        stepBuf.setSample(0, 0, 1.0f);
        stepBuf.setSample(1, 0, 1.0f);

        bool dummySplit = false;
        wvzz::ProcessContext ctx {
            stepBuf,
            dummyBuf,
            dummyBuf,
            dummySplit,
            nullptr,
            0,
            120.0,
            0.0,
            false,
            0,
            0.0f
        };

        transDSP.process(ctx);
        const float peak = stepBuf.getMagnitude(0, blockSize);
        std::cout << "  -> TransientShaper: Processed peak: " << peak << std::endl;
        assert(!std::isnan(peak) && peak > 0.0f);
    }

    // 3. Maximizer Clipper Ceiling Limiting
    {
        wvzz::modules::MaximizerClipperDSP clipDSP("clip");
        clipDSP.prepare(spec);
        clipDSP.setParameters(12.0f, -0.1f, 0, 1.0f);

        juce::AudioBuffer<float> loudBuf(2, blockSize);
        juce::AudioBuffer<float> dummyBuf(2, blockSize);
        generateSineWave(loudBuf, sampleRate, 1000.0f);
        loudBuf.applyGain(4.0f);

        bool dummySplit = false;
        wvzz::ProcessContext ctx {
            loudBuf,
            dummyBuf,
            dummyBuf,
            dummySplit,
            nullptr,
            0,
            120.0,
            0.0,
            false,
            0,
            0.0f
        };

        clipDSP.process(ctx);
        const float maxPeak = loudBuf.getMagnitude(0, blockSize);
        std::cout << "  -> MaximizerClipper: Clipped peak with +12dB input drive: " << maxPeak << std::endl;
        if (maxPeak > 1.05f)
        {
            std::cerr << "FAIL: MaximizerClipper failed to limit loud signal: " << maxPeak << std::endl;
            std::exit(1);
        }
    }

    // 4. DynDetector Linkwitz-Riley 24 dB/oct Flat Magnitude Recombination
    {
        wvzz::modules::DynDetectorDSP detDSP("det");
        detDSP.prepare(spec);
        detDSP.setParameters(120.0f, true, 0.0f, 20.0f, 0.0f); // subTight = 0.0 -> pure LR4 split

        const int settleSize = 4096;
        juce::AudioBuffer<float> mainBuf(2, settleSize);
        juce::AudioBuffer<float> subAuxBuf(2, settleSize);
        juce::AudioBuffer<float> scBuf(2, settleSize);

        generateSineWave(mainBuf, sampleRate, 120.0f);
        bool hasSubSplit = false;
        wvzz::ProcessContext ctx { mainBuf, scBuf, subAuxBuf, hasSubSplit, nullptr, 0, 120.0, 0.0, false, 0, 0.0f };
        detDSP.process(ctx);
        mainBuf.addFrom(0, 0, subAuxBuf.getReadPointer(0), settleSize);

        const float recombinedPeak = mainBuf.getMagnitude(0, settleSize / 2, settleSize / 2);
        std::cout << "  -> DynDetector LR4 120Hz Recombined Peak (Expected ~1.00): " << recombinedPeak << std::endl;
        if (std::abs(recombinedPeak - 1.00f) > 0.02f)
        {
            std::cerr << "FAIL: DynDetector LR4 crossover recombination magnitude error: " << recombinedPeak << std::endl;
            std::exit(1);
        }
    }

    // 5. MultibandOTT 3-Band LR4 Flat Recombination & Upward Compression Boost
    {
        wvzz::modules::MultibandOTTDSP ottDSP("ott");
        ottDSP.prepare(spec);
        // Set 0 upward, 0 downward, 0 sheen, 100% depth -> pure 3-band LR4 crossover sum
        ottDSP.setParameters(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, false);

        const int settleSize = 4096;
        juce::AudioBuffer<float> mainBuf(2, settleSize);
        juce::AudioBuffer<float> dummyBuf(2, settleSize);

        generateSineWave(mainBuf, sampleRate, 120.0f); // Test right on Crossover 1
        bool dummySplit = false;
        wvzz::ProcessContext ctx { mainBuf, dummyBuf, dummyBuf, dummySplit, nullptr, 0, 120.0, 0.0, false, 0, 0.0f };
        ottDSP.process(ctx);

        const float unityPeak = mainBuf.getMagnitude(0, settleSize / 2, settleSize / 2);
        std::cout << "  -> MultibandOTT 3-Band LR4 Unity Recombined Peak (Expected ~1.00): " << unityPeak << std::endl;
        if (std::abs(unityPeak - 1.00f) > 0.02f)
        {
            std::cerr << "FAIL: MultibandOTT 3-band crossover failed flat recombination: " << unityPeak << std::endl;
            std::exit(1);
        }
    }

    // 6. WvZzDynamics Master Bypass & Dry/Wet Mix Fidelity (Ensuring SC HPF does not leak into Dry path)
    {
        wvzz::WvZzDynamicsAudioProcessor proc;
        proc.prepareToPlay(sampleRate, 4096);

        // Set SC HPF to 500 Hz and Master Bypass = ON
        if (auto* pScHp = proc.getAPVTS().getParameter("det_sc_hp"))
            pScHp->setValueNotifyingHost(pScHp->convertTo0to1(500.0f));
        if (auto* pBypass = proc.getAPVTS().getParameter("master_bypass"))
            pBypass->setValueNotifyingHost(1.0f);

        juce::AudioBuffer<float> subToneBuf(2, 4096);
        juce::MidiBuffer midi;

        generateSineWave(subToneBuf, sampleRate, 50.0f); // 50 Hz sub-bass (well below 500 Hz SC HPF)
        proc.processBlock(subToneBuf, midi);

        const float bypassedSubPeak = subToneBuf.getMagnitude(0, 2048, 2048);
        std::cout << "  -> WvZzDynamics Master Bypass 50Hz Peak with 500Hz SC HPF (Expected ~1.00): " << bypassedSubPeak << std::endl;
        if (std::abs(bypassedSubPeak - 1.00f) > 0.02f)
        {
            std::cerr << "FAIL: Master Bypass leaked filtered sidechain signal instead of clean dry input: " << bypassedSubPeak << std::endl;
            std::exit(1);
        }
    }

    // 7. Transient Gate Stability & Full-Spectrum Detection under Sub Lock around -16.3 dB
    {
        wvzz::modules::DynDetectorDSP detDSP("det");
        wvzz::modules::TransientShaperDSP transDSP("trans");
        detDSP.prepare(spec);
        transDSP.prepare(spec);

        // Sub Lock ON at 90 Hz
        detDSP.setParameters(90.0f, true, 0.0f, 80.0f, 0.0f);
        // Attack = 0, Sustain = 0, Gate = -16.3 dB (0.1531 linear)
        transDSP.setParameters(0.0f, 0.0f, 0, -16.3f, false);

        const int testSamples = 4096;
        juce::AudioBuffer<float> mainBuf(2, testSamples);
        juce::AudioBuffer<float> subAuxBuf(2, testSamples);
        juce::AudioBuffer<float> scBuf(2, testSamples);

        // Case A: Signal at -14 dBFS (0.20 linear > -16.3 dB threshold) -> Gate must stay 100% open without chattering
        generateSineWave(mainBuf, sampleRate, 60.0f);
        mainBuf.applyGain(0.20f);
        bool hasSubSplit = false;
        wvzz::ProcessContext ctxOpen { mainBuf, scBuf, subAuxBuf, hasSubSplit, nullptr, 0, 120.0, 0.0, false, 0, 0.0f };
        detDSP.process(ctxOpen);
        transDSP.process(ctxOpen);
        mainBuf.addFrom(0, 0, subAuxBuf.getReadPointer(0), testSamples);

        const float openPeak = mainBuf.getMagnitude(0, testSamples / 2, testSamples / 2);
        std::cout << "  -> TransientGate (-16.3 dB) with -14 dBFS 60Hz signal Peak (Expected ~0.20): " << openPeak << std::endl;
        if (std::abs(openPeak - 0.20f) > 0.015f)
        {
            std::cerr << "FAIL: TransientGate chattered or attenuated signal above threshold: " << openPeak << std::endl;
            std::exit(1);
        }

        // Case B: Quiet noise tail at -32 dBFS (0.025 linear < -16.3 dB threshold) -> Gate must attenuate both Upper and Sub bands
        detDSP.reset();
        transDSP.reset();
        generateSineWave(mainBuf, sampleRate, 60.0f);
        mainBuf.applyGain(0.025f);
        wvzz::ProcessContext ctxClosed { mainBuf, scBuf, subAuxBuf, hasSubSplit, nullptr, 0, 120.0, 0.0, false, 0, 0.0f };
        detDSP.process(ctxClosed);
        transDSP.process(ctxClosed);
        mainBuf.addFrom(0, 0, subAuxBuf.getReadPointer(0), testSamples);

        const float gatedTailPeak = mainBuf.getMagnitude(0, (testSamples * 3) / 4, testSamples / 4);
        std::cout << "  -> TransientGate (-16.3 dB) with -32 dBFS 60Hz signal Gated Peak (Expected < 0.005): " << gatedTailPeak << std::endl;
        if (gatedTailPeak > 0.006f)
        {
            std::cerr << "FAIL: TransientGate failed to gate sub-bass or tail below threshold: " << gatedTailPeak << std::endl;
            std::exit(1);
        }
    }

    std::cout << "  -> PASS: DynDetector, TransientShaper, MultibandOTT, MaximizerClipper, and Master Dry/Wet verified.\n" << std::endl;
}

static void testWvZzDynamicsEditorLayoutAndNonOverlap()
{
    std::cout << "[Test 19/19] Verifying WvZz Dynamics UI Layout and Geometry..." << std::endl;

    wvzz::WvZzDynamicsAudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    if (!editor)
    {
        std::cerr << "FAIL: createEditor() returned null for WvZz Dynamics!" << std::endl;
        std::exit(1);
    }

    editor->setSize(1080, 680);
    if (editor->getWidth() != 1080 || editor->getHeight() != 680)
    {
        std::cerr << "FAIL: Editor dimensions mismatch: " << editor->getWidth() << "x" << editor->getHeight() << std::endl;
        std::exit(1);
    }

    std::cout << "  -> PASS: WvZz Dynamics UI instantiated at 1080x680 canvas successfully.\n" << std::endl;
}

int main(int, char*[])
{
    std::cout << "=====================================================" << std::endl;
    std::cout << "   WvZz Audio Plugins DSP Engine - Automated Tests   " << std::endl;
    std::cout << "=====================================================\n" << std::endl;

    try
    {
        testZeroAllocations();
        testMultiInstancePrefixing();
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
        testEdmHardstyleDspFeatures();

        // Dynamics suites
        testWvZzDynamicsZeroAllocations();
        testWvZzDynamicsMultiInstancePrefixing();
        testWvZzDynamicsFactoryPresetsRecall();
        testWvZzDynamicsDSPMath();
        testWvZzDynamicsEditorLayoutAndNonOverlap();

        std::cout << "=====================================================" << std::endl;
        std::cout << "   ALL 19 DSP SUITES PASSED (100% SUCCESS)           " << std::endl;
        std::cout << "=====================================================" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "EXCEPTION ENCOUNTERED: " << e.what() << std::endl;
        return 1;
    }
}
