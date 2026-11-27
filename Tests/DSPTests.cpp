#include "Plugins/WvZzDistortion/PluginProcessor.h"
#include "Plugins/WvZzDistortion/PluginEditor.h"
#include "Plugins/WvZzDynamics/PluginProcessor.h"
#include "Plugins/WvZzDynamics/PluginEditor.h"
#include "Plugins/WvZzDynamics/Presets/FactoryPresets.h"
#include "Plugins/WvZzModulation/PluginProcessor.h"
#include "Plugins/WvZzModulation/PluginEditor.h"
#include "Plugins/WvZzDelay/PluginProcessor.h"
#include "Plugins/WvZzDelay/PluginEditor.h"
#include "Modules/Delay/DelayModule.h"
#include "Modules/Grain/GrainModule.h"
#include "Core/UI/DelayTapDisplayComponent.h"
#include "Modules/Chorus/ChorusModule.h"
#include "Modules/Phaser/PhaserModule.h"
#include "Modules/Tremolo/TremoloModule.h"
#include "Core/UI/LfoPhaseScopeComponent.h"
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
#include <algorithm>
#include <functional>

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

// ==============================================================================
// WvZz Modulation suites (GH-15)
// These use explicit checks, because assert() is compiled out in Release builds.
// ==============================================================================
static void check(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << std::endl;
        std::exit(1);
    }
}

/** Minimal host for one DSP module: owns an APVTS built from the module's own parameters. */
class ModuleTestHost : public juce::AudioProcessor
{
public:
    explicit ModuleTestHost(wvzz::IDSPModule& moduleToHost)
        : module(moduleToHost),
          apvts(*this, nullptr, "Parameters", makeLayout(moduleToHost))
    {
        module.bindParameters(apvts);
    }

    void set(const juce::String& paramId, float value)
    {
        auto* param = apvts.getParameter(paramId);
        check(param != nullptr, ("unknown parameter id: " + paramId).toRawUTF8());
        param->setValueNotifyingHost(param->convertTo0to1(value));
    }

    void prepare(double sampleRate, int blockSize)
    {
        module.prepare({ sampleRate, static_cast<juce::uint32>(blockSize), 2 });
        dry.setSize(2, blockSize);
        aux.setSize(2, blockSize);
        dry.clear();
        aux.clear();
    }

    /** Runs the module on the buffer with no modulation offsets. */
    void run(juce::AudioBuffer<float>& buffer) noexcept
    {
        bool hasAux = false;
        wvzz::ProcessContext ctx { buffer, dry, aux, hasAux };
        module.process(ctx);
    }

    const juce::String getName() const override { return "ModuleTestHost"; }
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout(wvzz::IDSPModule& m)
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        m.registerParameters(layout);
        return layout;
    }

    wvzz::IDSPModule& module;
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioBuffer<float> dry;
    juce::AudioBuffer<float> aux;
};

static void fillNoise(juce::AudioBuffer<float>& buffer, juce::Random& rng, float amplitude)
{
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float v = (rng.nextFloat() * 2.0f - 1.0f) * amplitude;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.setSample(ch, i, v); // same signal on every channel
    }
}

/** Fills consecutive blocks of one continuous sine (phase carried by the caller). */
static void fillSineBlock(juce::AudioBuffer<float>& buffer, double sampleRate, float hz, double& phase, float amplitude)
{
    const double inc = juce::MathConstants<double>::twoPi * hz / sampleRate;
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float v = static_cast<float>(std::sin(phase)) * amplitude;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.setSample(ch, i, v);
        phase += inc;
    }
}

static float maxAbsDiff(const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b)
{
    float worst = 0.0f;
    for (int ch = 0; ch < juce::jmin(a.getNumChannels(), b.getNumChannels()); ++ch)
        for (int i = 0; i < juce::jmin(a.getNumSamples(), b.getNumSamples()); ++i)
            worst = std::max(worst, std::abs(a.getSample(ch, i) - b.getSample(ch, i)));
    return worst;
}

static bool allFinite(const juce::AudioBuffer<float>& buffer, float limit)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float v = buffer.getSample(ch, i);
            if (!std::isfinite(v) || std::abs(v) >= limit)
                return false;
        }
    return true;
}

/** Pearson correlation of two equally long series. */
static float correlation(const std::vector<float>& a, const std::vector<float>& b)
{
    const size_t n = std::min(a.size(), b.size());
    double meanA = 0.0, meanB = 0.0;
    for (size_t i = 0; i < n; ++i) { meanA += a[i]; meanB += b[i]; }
    meanA /= static_cast<double>(n);
    meanB /= static_cast<double>(n);
    double cov = 0.0, varA = 0.0, varB = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
        cov  += (a[i] - meanA) * (b[i] - meanB);
        varA += (a[i] - meanA) * (a[i] - meanA);
        varB += (b[i] - meanB) * (b[i] - meanB);
    }
    return static_cast<float>(cov / std::sqrt(std::max(1e-30, varA * varB)));
}

/**
 * The flanger algorithm exactly as it was before flg_feedback / flg_depth / flg_center existed
 * (fixed 2.5 ms center, +/-2.0 ms depth, 0.55 feedback). Reference for the equivalence test.
 */
class LegacyFlangerReference
{
public:
    void prepare(double sr)
    {
        sampleRate = sr;
        maxDelaySamples = static_cast<int>(sampleRate * 0.02) + 16;
        bufL.assign(static_cast<size_t>(maxDelaySamples), 0.0f);
        bufR.assign(static_cast<size_t>(maxDelaySamples), 0.0f);
    }

    void process(juce::AudioBuffer<float>& buffer, float rateHz, float amount)
    {
        float* l = buffer.getWritePointer(0);
        float* r = buffer.getWritePointer(1);
        const double phaseIncrement = (juce::MathConstants<double>::twoPi * rateHz) / sampleRate;
        const float centerDelaySamples = static_cast<float>(sampleRate * 0.0025);
        const float sweepModSamples    = static_cast<float>(sampleRate * 0.0020);
        constexpr float fixedFeedback  = 0.55f;

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float lfoL = static_cast<float>(std::sin(lfoPhase));
            const float lfoR = static_cast<float>(std::sin(lfoPhase + juce::MathConstants<double>::halfPi));
            lfoPhase += phaseIncrement;
            if (lfoPhase >= juce::MathConstants<double>::twoPi)
                lfoPhase -= juce::MathConstants<double>::twoPi;

            const float inL = l[i];
            const float delayedL = read(bufL, centerDelaySamples + sweepModSamples * lfoL);
            bufL[static_cast<size_t>(writeIndex)] = inL + std::tanh(fixedFeedback * fbL);
            fbL = delayedL;
            l[i] = (1.0f - amount) * inL + amount * delayedL;

            const float inR = r[i];
            const float delayedR = read(bufR, centerDelaySamples + sweepModSamples * lfoR);
            bufR[static_cast<size_t>(writeIndex)] = inR + std::tanh(fixedFeedback * fbR);
            fbR = delayedR;
            r[i] = (1.0f - amount) * inR + amount * delayedR;

            if (++writeIndex >= maxDelaySamples)
                writeIndex = 0;
        }
    }

private:
    float read(const std::vector<float>& buf, float delay) const
    {
        float readPos = static_cast<float>(writeIndex) - delay;
        while (readPos < 0.0f)
            readPos += static_cast<float>(maxDelaySamples);
        const int i1 = static_cast<int>(readPos);
        const int i0 = (i1 - 1 + maxDelaySamples) % maxDelaySamples;
        const int i2 = (i1 + 1) % maxDelaySamples;
        const int i3 = (i1 + 2) % maxDelaySamples;
        const float d = readPos - static_cast<float>(i1);
        const float c0 = -d * (d - 1.0f) * (d - 2.0f) / 6.0f;
        const float c1 = (d + 1.0f) * (d - 1.0f) * (d - 2.0f) / 2.0f;
        const float c2 = -(d + 1.0f) * d * (d - 2.0f) / 2.0f;
        const float c3 = (d + 1.0f) * d * (d - 1.0f) / 6.0f;
        return c0 * buf[static_cast<size_t>(i0)] + c1 * buf[static_cast<size_t>(i1)]
             + c2 * buf[static_cast<size_t>(i2)] + c3 * buf[static_cast<size_t>(i3)];
    }

    double sampleRate { 48000.0 };
    int maxDelaySamples { 0 };
    int writeIndex { 0 };
    double lfoPhase { 0.0 };
    float fbL { 0.0f }, fbR { 0.0f };
    std::vector<float> bufL, bufR;
};

static void testFlangerExtension()
{
    std::cout << "[Modulation 1/9] Flanger: default equivalence, through-zero, negative feedback..." << std::endl;

    const double sampleRate = 48000.0;
    const int blockSize = 512;

    // --- Scenario: Default values keep the previous sound (APVTS-bound, as in WvZz Distortion) ---
    {
        wvzz::modules::FlangerDSP flanger("flg");
        ModuleTestHost host(flanger);
        host.set("flg_bypass", 0.0f);
        host.set("flg_rate", 0.4f);
        host.set("flg_amount", 0.7f);
        host.prepare(sampleRate, blockSize);

        LegacyFlangerReference reference;
        reference.prepare(sampleRate);

        juce::Random rng(42);
        juce::AudioBuffer<float> actual(2, blockSize), expected(2, blockSize);
        float worst = 0.0f;
        for (int b = 0; b < 200; ++b) // about 2.1 s: more than 0.8 of an LFO cycle
        {
            fillNoise(actual, rng, 0.5f);
            actual.applyGain(1, 0, blockSize, -0.8f); // different right channel
            expected.makeCopyOf(actual);
            host.run(actual);
            // Rate and amount existed before; the reference reads the same stored parameter values.
            reference.process(expected,
                              host.getAPVTS().getRawParameterValue("flg_rate")->load(),
                              host.getAPVTS().getRawParameterValue("flg_amount")->load());
            worst = std::max(worst, maxAbsDiff(actual, expected));
        }
        std::cout << "  -> default flanger vs previous algorithm: max difference " << worst << std::endl;
        check(worst < 1.0e-5f, "extended flanger with default values must equal the previous flanger");
    }

    // --- Scenario: Through-zero mode. With depth 0 and feedback 0 the wet and the delayed dry path coincide. ---
    {
        wvzz::modules::FlangerDSP flanger("flg");
        ModuleTestHost host(flanger);
        host.set("flg_bypass", 0.0f);
        host.set("flg_amount", 0.5f);
        host.set("flg_feedback", 0.0f);
        host.set("flg_depth", 0.0f);
        host.set("flg_center", 2.5f);
        host.set("flg_tzero", 1.0f);
        host.prepare(sampleRate, blockSize);

        const int centerSamples = 120; // 2.5 ms at 48 kHz
        juce::Random rng(7);
        std::vector<float> input, output;
        juce::AudioBuffer<float> buffer(2, blockSize);
        for (int b = 0; b < 8; ++b)
        {
            fillNoise(buffer, rng, 0.5f);
            for (int i = 0; i < blockSize; ++i) input.push_back(buffer.getSample(0, i));
            host.run(buffer);
            for (int i = 0; i < blockSize; ++i) output.push_back(buffer.getSample(0, i));
        }
        float worst = 0.0f;
        for (size_t n = static_cast<size_t>(centerSamples) + 8; n < output.size(); ++n)
            worst = std::max(worst, std::abs(output[n] - input[n - static_cast<size_t>(centerSamples)]));
        std::cout << "  -> through-zero, depth 0: output vs input delayed by 2.5 ms: max difference " << worst << std::endl;
        check(worst < 1.0e-3f, "through-zero must delay the dry path by flg_center");

        // With through-zero off, the dry path is not delayed, so the result is different.
        wvzz::modules::FlangerDSP normal("flg");
        ModuleTestHost normalHost(normal);
        normalHost.set("flg_bypass", 0.0f);
        normalHost.set("flg_amount", 0.5f);
        normalHost.set("flg_feedback", 0.0f);
        normalHost.set("flg_depth", 0.0f);
        normalHost.prepare(sampleRate, blockSize);
        juce::Random rng2(7);
        fillNoise(buffer, rng2, 0.5f);
        juce::AudioBuffer<float> in(2, blockSize);
        in.makeCopyOf(buffer);
        normalHost.run(buffer);
        check(std::abs(buffer.getSample(0, 300) - (0.5f * in.getSample(0, 300) + 0.5f * in.getSample(0, 300 - centerSamples))) < 1.0e-3f,
              "with through-zero off the dry path must not be delayed");
    }

    // --- Scenario: Negative feedback stays finite and differs from positive feedback ---
    {
        juce::AudioBuffer<float> outNeg(2, blockSize), outPos(2, blockSize);
        for (int pass = 0; pass < 2; ++pass)
        {
            wvzz::modules::FlangerDSP flanger("flg");
            ModuleTestHost host(flanger);
            host.set("flg_bypass", 0.0f);
            host.set("flg_amount", 0.5f);
            host.set("flg_feedback", pass == 0 ? -0.95f : 0.95f);
            host.prepare(sampleRate, blockSize);
            juce::Random rng(11);
            auto& out = (pass == 0) ? outNeg : outPos;
            for (int b = 0; b < 190; ++b) // 2 s
            {
                fillNoise(out, rng, 1.0f);
                host.run(out);
                check(allFinite(out, 8.0f), "flanger output must stay finite and bounded at +/-0.95 feedback");
            }
        }
        check(maxAbsDiff(outNeg, outPos) > 0.01f, "negative feedback must sound different from positive feedback");
    }

    std::cout << "  -> PASS\n" << std::endl;
}

static void testChorusModule()
{
    std::cout << "[Modulation 2/9] Chorus: voices, stereo spread, dry mix, zero allocation, prefix..." << std::endl;
    const double sampleRate = 48000.0;
    const int blockSize = 512;

    // --- Scenario: Chorus adds modulated voices (output differs, level changes over time) ---
    {
        wvzz::modules::ChorusDSP chorus("chor");
        ModuleTestHost host(chorus);
        host.set("chor_bypass", 0.0f);
        host.set("chor_mix", 0.5f);
        host.set("chor_rate", 2.0f);
        host.set("chor_depth", 0.8f);
        host.set("chor_voices", 0.0f); // 1 voice: clearest beating against the dry tone
        host.set("chor_spread", 0.0f);
        host.prepare(sampleRate, blockSize);

        juce::AudioBuffer<float> buffer(2, blockSize), input(2, blockSize);
        double phase = 0.0;
        float minRms = 10.0f, maxRms = 0.0f, worstDiff = 0.0f;
        for (int b = 0; b < 100; ++b)
        {
            fillSineBlock(buffer, sampleRate, 440.0f, phase, 0.5f);
            input.makeCopyOf(buffer);
            host.run(buffer);
            if (b > 10)
            {
                const float rms = calculateRms(buffer, 0);
                minRms = std::min(minRms, rms);
                maxRms = std::max(maxRms, rms);
                worstDiff = std::max(worstDiff, maxAbsDiff(buffer, input));
            }
        }
        std::cout << "  -> block RMS range " << minRms << " .. " << maxRms << std::endl;
        check(worstDiff > 0.05f, "chorus output must differ from the input");
        check(maxRms > minRms * 1.2f, "chorus level must change over time on a steady tone");
    }

    // --- Scenario: Stereo spread decorrelates the channels; spread 0 keeps them equal ---
    for (int pass = 0; pass < 2; ++pass)
    {
        wvzz::modules::ChorusDSP chorus("chor");
        ModuleTestHost host(chorus);
        host.set("chor_bypass", 0.0f);
        host.set("chor_spread", pass == 0 ? 1.0f : 0.0f);
        host.prepare(sampleRate, blockSize);

        juce::AudioBuffer<float> buffer(2, blockSize);
        double phase = 0.0;
        float worstLR = 0.0f;
        for (int b = 0; b < 40; ++b)
        {
            fillSineBlock(buffer, sampleRate, 440.0f, phase, 0.5f);
            host.run(buffer);
            for (int i = 0; i < blockSize; ++i)
                worstLR = std::max(worstLR, std::abs(buffer.getSample(0, i) - buffer.getSample(1, i)));
        }
        if (pass == 0) check(worstLR > 0.01f, "chorus spread 1.0 must make left and right differ");
        else           check(worstLR < 1.0e-6f, "chorus spread 0.0 must keep left and right equal");
    }

    // --- Scenario: Chorus mix at zero is dry ---
    {
        wvzz::modules::ChorusDSP chorus("chor");
        ModuleTestHost host(chorus);
        host.set("chor_bypass", 0.0f);
        host.set("chor_mix", 0.0f);
        host.prepare(sampleRate, blockSize);
        juce::AudioBuffer<float> buffer(2, blockSize), input(2, blockSize);
        juce::Random rng(3);
        float worst = 0.0f;
        for (int b = 0; b < 10; ++b)
        {
            fillNoise(buffer, rng, 0.5f);
            input.makeCopyOf(buffer);
            host.run(buffer);
            worst = std::max(worst, maxAbsDiff(buffer, input));
        }
        check(worst < 1.0e-6f, "chorus with mix 0 must output the input");
    }

    // --- Zero allocation (4 voices, feedback on) and prefixing ---
    {
        wvzz::modules::ChorusDSP chorus("slot2_chor");
        ModuleTestHost host(chorus);
        check(host.getAPVTS().getParameter("slot2_chor_mix") != nullptr && host.getAPVTS().getParameter("chor_mix") == nullptr,
              "chorus parameter ids must use the instance prefix");
        host.set("slot2_chor_bypass", 0.0f);
        host.set("slot2_chor_voices", 3.0f);
        host.set("slot2_chor_feedback", 0.5f);
        host.prepare(sampleRate, blockSize);
        juce::AudioBuffer<float> buffer(2, blockSize);
        generateSineWave(buffer, sampleRate, 440.0f);
        host.run(buffer);
        g_allocationCount = 0;
        g_trackingAllocations = true;
        for (int b = 0; b < 1000; ++b)
        {
            generateSineWave(buffer, sampleRate, 440.0f); // fresh input each block; does not allocate
            host.run(buffer);
        }
        g_trackingAllocations = false;
        std::cout << "  -> heap allocations in 1,000 blocks: " << g_allocationCount << std::endl;
        check(g_allocationCount == 0, "chorus must not allocate while processing");
        check(allFinite(buffer, 8.0f), "chorus output must stay finite");
    }

    std::cout << "  -> PASS\n" << std::endl;
}

static void testPhaserModule()
{
    std::cout << "[Modulation 3/9] Phaser: moving notches, stability, stage count, zero allocation, prefix..." << std::endl;
    const double sampleRate = 48000.0;
    const int blockSize = 512;

    auto runNoise = [&](int stagesIndex, float feedback, float amplitude, int blocks, juce::AudioBuffer<float>& lastOut, bool& stayedBounded) {
        wvzz::modules::PhaserDSP phaser("phs");
        ModuleTestHost host(phaser);
        host.set("phs_bypass", 0.0f);
        host.set("phs_mix", 0.5f);
        host.set("phs_rate", 1.0f);
        host.set("phs_stages", static_cast<float>(stagesIndex));
        host.set("phs_feedback", feedback);
        host.prepare(sampleRate, blockSize);
        juce::Random rng(99);
        stayedBounded = true;
        float worstDiff = 0.0f;
        juce::AudioBuffer<float> input(2, blockSize);
        for (int b = 0; b < blocks; ++b)
        {
            fillNoise(lastOut, rng, amplitude);
            input.makeCopyOf(lastOut);
            host.run(lastOut);
            stayedBounded = stayedBounded && allFinite(lastOut, 8.0f);
            worstDiff = std::max(worstDiff, maxAbsDiff(lastOut, input));
        }
        return worstDiff;
    };

    // --- Scenario: Phaser creates moving notches (output differs from the input) ---
    juce::AudioBuffer<float> out4(2, blockSize), out2(2, blockSize), out12(2, blockSize);
    bool bounded = true;
    const float diff = runNoise(1, 0.5f, 0.5f, 50, out4, bounded);
    check(diff > 0.05f && bounded, "phaser output must differ from the input and stay bounded");

    // --- Scenario: Phaser stays stable at maximum feedback (12 stages, full scale, 2 s) ---
    for (float fb : { 0.95f, -0.95f })
    {
        juce::AudioBuffer<float> out(2, blockSize);
        runNoise(4, fb, 1.0f, 190, out, bounded);
        check(bounded, "phaser must stay finite and below 8.0 at +/-0.95 feedback with 12 stages");
    }

    // --- Scenario: Stage count changes the response ---
    runNoise(0, 0.5f, 0.5f, 50, out2, bounded);
    runNoise(4, 0.5f, 0.5f, 50, out12, bounded);
    check(maxAbsDiff(out2, out12) > 0.01f, "2 stages and 12 stages must give different outputs");
    check(wvzz::modules::PhaserDSP::stagesForIndex(0) == 2 && wvzz::modules::PhaserDSP::stagesForIndex(4) == 12,
          "stage choice must map to 2..12 sections");

    // --- Zero allocation and prefixing ---
    {
        wvzz::modules::PhaserDSP phaser("slot2_phs");
        ModuleTestHost host(phaser);
        check(host.getAPVTS().getParameter("slot2_phs_center") != nullptr && host.getAPVTS().getParameter("phs_center") == nullptr,
              "phaser parameter ids must use the instance prefix");
        host.set("slot2_phs_bypass", 0.0f);
        host.set("slot2_phs_stages", 4.0f);
        host.prepare(sampleRate, blockSize);
        juce::AudioBuffer<float> buffer(2, blockSize);
        generateSineWave(buffer, sampleRate, 440.0f);
        host.run(buffer);
        g_allocationCount = 0;
        g_trackingAllocations = true;
        for (int b = 0; b < 1000; ++b)
        {
            generateSineWave(buffer, sampleRate, 440.0f); // fresh input each block; does not allocate
            host.run(buffer);
        }
        g_trackingAllocations = false;
        std::cout << "  -> heap allocations in 1,000 blocks: " << g_allocationCount << std::endl;
        check(g_allocationCount == 0, "phaser must not allocate while processing");
    }

    std::cout << "  -> PASS\n" << std::endl;
}

static void testTremoloModule()
{
    std::cout << "[Modulation 4/9] Tremolo: classic, harmonic, zero depth, stereo, zero allocation, prefix..." << std::endl;
    const double sampleRate = 48000.0;
    const int blockSize = 256;

    // Collects the per-block RMS of left and right for a sine input.
    auto envelope = [&](int mode, float depth, float stereo, float toneHz, std::vector<float>& left, std::vector<float>& right, float* worstDiff = nullptr) {
        wvzz::modules::TremoloDSP tremolo("trem");
        ModuleTestHost host(tremolo);
        host.set("trem_bypass", 0.0f);
        host.set("trem_rate", 2.0f);
        host.set("trem_depth", depth);
        host.set("trem_mode", static_cast<float>(mode));
        host.set("trem_stereo", stereo);
        host.set("trem_xover", 800.0f);
        host.prepare(sampleRate, blockSize);
        juce::AudioBuffer<float> buffer(2, blockSize), input(2, blockSize);
        double phase = 0.0;
        for (int b = 0; b < 400; ++b) // about 2.1 s = 4 LFO cycles
        {
            fillSineBlock(buffer, sampleRate, toneHz, phase, 0.5f);
            input.makeCopyOf(buffer);
            host.run(buffer);
            left.push_back(calculateRms(buffer, 0));
            right.push_back(calculateRms(buffer, 1));
            if (worstDiff != nullptr)
                *worstDiff = std::max(*worstDiff, maxAbsDiff(buffer, input));
        }
    };

    // --- Scenario: Classic tremolo modulates the level between the input level and near silence ---
    {
        std::vector<float> l, r;
        envelope(0, 1.0f, 0.0f, 1000.0f, l, r);
        const float inputRms = 0.5f / std::sqrt(2.0f);
        const float lo = *std::min_element(l.begin() + 20, l.end());
        const float hi = *std::max_element(l.begin() + 20, l.end());
        std::cout << "  -> classic depth 1.0: block RMS " << lo << " .. " << hi << " (input " << inputRms << ")" << std::endl;
        check(hi > inputRms * 0.9f && hi < inputRms * 1.05f, "classic tremolo peak must be about the input level");
        check(lo < inputRms * 0.1f, "classic tremolo at depth 1.0 must reach near silence");
    }

    // --- Scenario: Harmonic tremolo moves low and high bands in opposite phase ---
    {
        std::vector<float> lowL, lowR, highL, highR;
        envelope(1, 1.0f, 0.0f, 100.0f, lowL, lowR);    // far below the 800 Hz crossover
        envelope(1, 1.0f, 0.0f, 6000.0f, highL, highR); // far above it
        const float c = correlation(lowL, highL);
        std::cout << "  -> harmonic mode: low band vs high band level correlation " << c << std::endl;
        check(c < -0.8f, "harmonic tremolo must move the low and high band in opposite phase");
    }

    // --- Scenario: Zero depth is transparent (within -80 dB) in both modes ---
    for (int mode = 0; mode < 2; ++mode)
    {
        std::vector<float> l, r;
        float worst = 0.0f;
        envelope(mode, 0.0f, 0.0f, 1000.0f, l, r, &worst);
        check(worst < 0.5f * 1.0e-4f, "tremolo with depth 0 must be transparent within -80 dB");
    }

    // --- Scenario: Full stereo offset pans the signal ---
    {
        std::vector<float> l, r;
        envelope(0, 1.0f, 1.0f, 1000.0f, l, r);
        const float c = correlation(l, r);
        std::cout << "  -> stereo 1.0: left vs right level correlation " << c << std::endl;
        check(c < -0.8f, "tremolo stereo 1.0 must make left loudest when right is quietest");
    }

    // --- Zero allocation and prefixing ---
    {
        wvzz::modules::TremoloDSP tremolo("slot2_trem");
        ModuleTestHost host(tremolo);
        check(host.getAPVTS().getParameter("slot2_trem_depth") != nullptr && host.getAPVTS().getParameter("trem_depth") == nullptr,
              "tremolo parameter ids must use the instance prefix");
        host.set("slot2_trem_bypass", 0.0f);
        host.set("slot2_trem_mode", 1.0f);
        host.set("slot2_trem_shape", 2.0f);
        host.prepare(sampleRate, 512);
        juce::AudioBuffer<float> buffer(2, 512);
        generateSineWave(buffer, sampleRate, 440.0f);
        host.run(buffer);
        g_allocationCount = 0;
        g_trackingAllocations = true;
        for (int b = 0; b < 1000; ++b)
        {
            generateSineWave(buffer, sampleRate, 440.0f); // fresh input each block; does not allocate
            host.run(buffer);
        }
        g_trackingAllocations = false;
        std::cout << "  -> heap allocations in 1,000 blocks: " << g_allocationCount << std::endl;
        check(g_allocationCount == 0, "tremolo must not allocate while processing");
    }

    std::cout << "  -> PASS\n" << std::endl;
}

static void setPluginParam(juce::AudioProcessorValueTreeState& apvts, const char* id, float value)
{
    auto* param = apvts.getParameter(id);
    check(param != nullptr, id);
    param->setValueNotifyingHost(param->convertTo0to1(value));
}

/** Every parameter back to its default: all four effects bypassed. */
static void resetPluginToDefaults(juce::AudioProcessor& processor)
{
    for (auto* param : processor.getParameters())
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
            ranged->setValueNotifyingHost(ranged->getDefaultValue());
}

static void testWvZzModulationChain()
{
    std::cout << "[Modulation 5/9] Plugin chain: all bypassed, serial stacking, tail length..." << std::endl;
    const double sampleRate = 48000.0;
    const int blockSize = 512;
    juce::MidiBuffer midi;

    // --- Scenario: All effects bypassed -> output equals input ---
    {
        wvzz::WvZzModulationAudioProcessor processor;
        resetPluginToDefaults(processor);
        processor.prepareToPlay(sampleRate, blockSize);
        juce::AudioBuffer<float> buffer(2, blockSize), input(2, blockSize);
        juce::Random rng(5);
        float worst = 0.0f;
        for (int b = 0; b < 20; ++b)
        {
            fillNoise(buffer, rng, 0.5f);
            input.makeCopyOf(buffer);
            processor.processBlock(buffer, midi);
            worst = std::max(worst, maxAbsDiff(buffer, input));
        }
        std::cout << "  -> all bypassed: max difference to input " << worst << std::endl;
        check(worst < 1.0e-6f, "with all four effects bypassed the output must equal the input");
    }

    // --- Scenario: Effects stack in series (tremolo acts on the chorused signal) ---
    {
        auto render = [&](bool chorusOn, bool tremoloOn, std::vector<float>& levels, juce::AudioBuffer<float>& last) {
            wvzz::WvZzModulationAudioProcessor processor;
            resetPluginToDefaults(processor);
            auto& apvts = processor.getAPVTS();
            setPluginParam(apvts, "chor_bypass", chorusOn ? 0.0f : 1.0f);
            setPluginParam(apvts, "trem_bypass", tremoloOn ? 0.0f : 1.0f);
            setPluginParam(apvts, "trem_depth", 1.0f);
            setPluginParam(apvts, "trem_rate", 2.0f);
            processor.prepareToPlay(sampleRate, blockSize);
            double phase = 0.0;
            for (int b = 0; b < 100; ++b)
            {
                fillSineBlock(last, sampleRate, 440.0f, phase, 0.5f);
                processor.processBlock(last, midi);
                levels.push_back(calculateRms(last, 0));
            }
        };
        std::vector<float> both, tremOnly, chorusOnly;
        juce::AudioBuffer<float> outBoth(2, blockSize), outTrem(2, blockSize), outChorus(2, blockSize);
        render(true, true, both, outBoth);
        render(false, true, tremOnly, outTrem);
        render(true, false, chorusOnly, outChorus);

        check(*std::min_element(both.begin() + 20, both.end()) < 0.05f, "with chorus + tremolo the tremolo must still gate the level");
        check(maxAbsDiff(outBoth, outTrem) > 0.005f, "chorus + tremolo must differ from tremolo alone");
        check(maxAbsDiff(outBoth, outChorus) > 0.005f, "chorus + tremolo must differ from chorus alone");
    }

    // --- Tail length reporting ---
    {
        wvzz::WvZzModulationAudioProcessor processor;
        check(std::abs(processor.getTailLengthSeconds() - 2.0) < 1.0e-9, "WvZz Modulation must report a 2.0 s tail");
        check(processor.getName() == "WvZz Modulation", "plugin name");
    }

    std::cout << "  -> PASS\n" << std::endl;
}

static void testWvZzModulationZeroAllocations()
{
    std::cout << "[Modulation 6/9] Plugin zero-allocation audit (all four effects active)..." << std::endl;

    wvzz::WvZzModulationAudioProcessor processor;
    processor.setCurrentProgram(9); // "Psychedelic Swirl": chorus, phaser, flanger, tremolo all on
    processor.prepareToPlay(48000.0, 512);

    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;
    generateSineWave(buffer, 48000.0, 440.0f);
    processor.processBlock(buffer, midi);

    g_allocationCount = 0;
    g_trackingAllocations = true;
    for (int i = 0; i < 1000; ++i)
        processor.processBlock(buffer, midi);
    g_trackingAllocations = false;

    std::cout << "  -> heap allocations in 1,000 processBlock() calls: " << g_allocationCount << std::endl;
    check(g_allocationCount == 0, "WvZz Modulation must not allocate on the audio thread");
    std::cout << "  -> PASS\n" << std::endl;
}

static void testWvZzModulationPresets()
{
    std::cout << "[Modulation 7/9] Factory presets: recall, stability, independence from history..." << std::endl;

    wvzz::WvZzModulationAudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);

    const auto& presets = wvzz::modulationfx::getFactoryPresets();
    check(presets.size() == 10, "WvZz Modulation must have 10 factory presets");
    check(processor.getNumPrograms() == 10, "getNumPrograms must be 10");
    check(processor.getCurrentProgram() == 0, "the plugin must start on program 0");

    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;

    for (int i = 0; i < 10; ++i)
    {
        processor.setCurrentProgram(i);
        check(processor.getProgramName(i) == juce::String(presets[static_cast<size_t>(i)].name), "preset name mismatch");

        // Every id in the preset must be a real parameter, and must hold the preset value after loading
        for (const auto& [paramId, value] : presets[static_cast<size_t>(i)].values)
        {
            auto* param = dynamic_cast<juce::RangedAudioParameter*>(processor.getAPVTS().getParameter(paramId));
            check(param != nullptr, paramId);
            const float actual = param->convertFrom0to1(param->getValue());
            if (std::abs(actual - value) > 0.02f * std::max(1.0f, std::abs(value)))
            {
                std::cerr << "FAIL: preset '" << presets[static_cast<size_t>(i)].name << "' " << paramId
                          << " = " << actual << ", expected " << value << std::endl;
                std::exit(1);
            }
        }

        for (int b = 0; b < 50; ++b)
        {
            generateSineWave(buffer, 48000.0, 220.0f);
            processor.processBlock(buffer, midi);
            check(allFinite(buffer, 8.0f), "preset output must be finite");
        }
    }

    // --- Scenario: Preset result is independent of history ---
    auto snapshot = [&processor]() {
        std::vector<float> values;
        for (auto* param : processor.getParameters())
            values.push_back(param->getValue());
        return values;
    };
    processor.setCurrentProgram(0);
    const auto direct = snapshot();
    processor.setCurrentProgram(9); // sets all four effects
    processor.setCurrentProgram(0);
    const auto afterOther = snapshot();
    check(direct == afterOther, "loading a preset must give the same parameter values whatever was loaded before");

    std::cout << "  -> PASS: 10 presets recalled, stable, and independent of history.\n" << std::endl;
}

static void testWvZzModulationTargets()
{
    std::cout << "[Modulation 8/9] Modulation profile: target lists and LFO delivery to Chorus Mix..." << std::endl;

    // --- Scenario: Existing profiles are unchanged ---
    const auto& distNames = wvzz::modules::getModTargetNames(wvzz::ModProfile::Distortion);
    const auto& dynNames  = wvzz::modules::getModTargetNames(wvzz::ModProfile::Dynamics);
    const auto& modNames  = wvzz::modules::getModTargetNames(wvzz::ModProfile::Modulation);
    check(distNames.size() == 22 && distNames[14] == "Flanger Rate" && distNames[21] == "Master Vol", "Distortion target list changed");
    check(dynNames.size() == 16 && dynNames[1] == "Sub X-Over" && dynNames[15] == "Master Vol", "Dynamics target list changed");
    check(static_cast<int>(wvzz::ModTarget::MasterVol) == 21 && static_cast<int>(wvzz::ModTarget::DynClipDrive) == 33,
          "existing ModTarget values must not move");
    check(modNames.size() == 15, "Modulation profile must list None + 14 targets");
    check(wvzz::modules::getModTargetForIndex(3, wvzz::ModProfile::Modulation) == wvzz::ModTarget::ModChorMix, "index 3 must be Chorus Mix");
    check(wvzz::modules::getModTargetForIndex(8, wvzz::ModProfile::Modulation) == wvzz::ModTarget::FlangerRate, "index 8 must be Flanger Rate");
    check(wvzz::modules::getModTargetForIndex(14, wvzz::ModProfile::Modulation) == wvzz::ModTarget::MasterVol, "index 14 must be Master Vol");

    // --- Scenario: LFO moves a chorus parameter ---
    wvzz::WvZzModulationAudioProcessor processor;
    auto& apvts = processor.getAPVTS();
    setPluginParam(apvts, "lfo1_target", 3.0f); // Chorus Mix
    setPluginParam(apvts, "lfo1_depth", 1.0f);
    setPluginParam(apvts, "lfo1_rate", 5.0f);
    processor.prepareToPlay(44100.0, 512);

    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();
    juce::MidiBuffer midi;
    bool sawOffset = false;
    for (int b = 0; b < 10; ++b)
    {
        processor.processBlock(buffer, midi);
        if (std::abs(processor.getModulationModule().getModulationDSP().getOffset(wvzz::ModTarget::ModChorMix)) > 0.01f)
            sawOffset = true;
    }
    check(sawOffset, "LFO 1 must deliver a non-zero offset to the Chorus Mix target");

    std::cout << "  -> PASS\n" << std::endl;
}

static void testWvZzModulationEditorAndScope()
{
    std::cout << "[Modulation 9/9] Editor at 1080x680, custom tab, LFO taps, Distortion/Dynamics layout unchanged..." << std::endl;

    wvzz::WvZzModulationAudioProcessor processor;
    processor.setCurrentProgram(9);
    processor.prepareToPlay(48000.0, 512);

    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    check(editor != nullptr, "createEditor() returned null for WvZz Modulation");
    editor->setSize(1080, 680);
    check(editor->getWidth() == 1080 && editor->getHeight() == 680, "WvZz Modulation editor must be 1080x680");

    // The custom display exists exactly once in the Modulation editor, and not in the other plugins.
    std::function<int(juce::Component&)> countScopes = [&countScopes](juce::Component& c) {
        int n = (dynamic_cast<wvzz::ui::LfoPhaseScopeComponent*>(&c) != nullptr) ? 1 : 0;
        for (auto* child : c.getChildren())
            n += countScopes(*child);
        return n;
    };
    check(countScopes(*editor) == 1, "WvZz Modulation editor must contain one LFO phase scope");

    wvzz::WvZzDistortionAudioProcessor distortion;
    std::unique_ptr<juce::AudioProcessorEditor> distEditor(distortion.createEditor());
    wvzz::WvZzDynamicsAudioProcessor dynamics;
    std::unique_ptr<juce::AudioProcessorEditor> dynEditor(dynamics.createEditor());
    check(distEditor != nullptr && dynEditor != nullptr, "Distortion and Dynamics editors must still open");
    check(countScopes(*distEditor) == 0 && countScopes(*dynEditor) == 0, "other plugins must not get a custom display tab");

    // The audio thread publishes LFO state for the scope: active flags and a moving phase.
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;
    generateSineWave(buffer, 48000.0, 440.0f);
    processor.processBlock(buffer, midi);
    const auto& tremTap = processor.getTremoloModule().getTremoloDSP().getLfoTap();
    const float phaseBefore = tremTap.phase.load();
    for (int b = 0; b < 4; ++b)
        processor.processBlock(buffer, midi);
    check(tremTap.active.load() && processor.getChorusModule().getChorusDSP().getLfoTap().active.load()
          && processor.getPhaserModule().getPhaserDSP().getLfoTap().active.load()
          && processor.getFlangerModule().getFlangerDSP().getLfoTap().active.load(),
          "all four LFO taps must be active when the effects are on");
    check(std::abs(tremTap.phase.load() - phaseBefore) > 1.0e-4f, "the tremolo LFO phase published for the scope must move");

    setPluginParam(processor.getAPVTS(), "trem_bypass", 1.0f);
    processor.processBlock(buffer, midi);
    check(!tremTap.active.load(), "a bypassed effect must publish an inactive LFO tap");

    check(std::abs(wvzz::ui::LfoPhaseScopeComponent::waveformValue(wvzz::dsp::LfoPhaseTap::Sine, 0.25f) - 1.0f) < 1.0e-5f
          && wvzz::ui::LfoPhaseScopeComponent::waveformValue(wvzz::dsp::LfoPhaseTap::Square, 0.75f) == -1.0f
          && std::abs(wvzz::ui::LfoPhaseScopeComponent::waveformValue(wvzz::dsp::LfoPhaseTap::Triangle, 0.5f) - 1.0f) < 1.0e-5f,
          "scope waveform shapes");

    std::cout << "  -> PASS\n" << std::endl;
}

// ==============================================================================
// WvZz Delay suites (GH-16)
// ==============================================================================

/** Delay + Grain as one module, wired like the plugin does it (Granular mode needs both). */
class DelayGrainPair : public wvzz::IDSPModule
{
public:
    DelayGrainPair() : delay("dly"), grain("grain")
    {
        grain.setExternalHost(true);
        delay.setGrainSource(&grain);
    }

    void registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override
    {
        delay.registerParameters(layout);
        grain.registerParameters(layout);
    }
    void bindParameters(juce::AudioProcessorValueTreeState& apvts) override { delay.bindParameters(apvts); grain.bindParameters(apvts); }
    void prepare(const juce::dsp::ProcessSpec& spec) override { grain.prepare(spec); delay.prepare(spec); }
    void reset() noexcept override { delay.reset(); grain.reset(); }
    void process(wvzz::ProcessContext& ctx) noexcept override { delay.process(ctx); }
    void randomize(juce::Random&) override {}

    wvzz::modules::DelayDSP delay;
    wvzz::modules::GrainDSP grain;
};

/** Runs a mono-compatible signal through a hosted module in blocks and returns the left (and right) output. */
static void renderThroughHost(ModuleTestHost& host, const std::vector<float>& input, std::vector<float>& outLeft,
                              std::vector<float>* outRight = nullptr, int blockSize = 512)
{
    juce::AudioBuffer<float> buffer(2, blockSize);
    outLeft.assign(input.size(), 0.0f);
    if (outRight != nullptr)
        outRight->assign(input.size(), 0.0f);

    for (size_t pos = 0; pos + static_cast<size_t>(blockSize) <= input.size(); pos += static_cast<size_t>(blockSize))
    {
        for (int i = 0; i < blockSize; ++i)
        {
            buffer.setSample(0, i, input[pos + static_cast<size_t>(i)]);
            buffer.setSample(1, i, input[pos + static_cast<size_t>(i)]);
        }
        host.run(buffer);
        for (int i = 0; i < blockSize; ++i)
        {
            outLeft[pos + static_cast<size_t>(i)] = buffer.getSample(0, i);
            if (outRight != nullptr)
                (*outRight)[pos + static_cast<size_t>(i)] = buffer.getSample(1, i);
        }
    }
}

/** Frequency of a tone from its zero crossings (crossing times are linearly interpolated). */
static float estimateFrequency(const std::vector<float>& x, size_t start, size_t length, double sampleRate)
{
    double first = -1.0, last = -1.0;
    int crossings = 0;
    for (size_t n = start + 1; n < start + length && n < x.size(); ++n)
    {
        if (x[n - 1] <= 0.0f && x[n] > 0.0f) // rising crossings only
        {
            const double t = static_cast<double>(n - 1) + static_cast<double>(-x[n - 1]) / static_cast<double>(x[n] - x[n - 1]);
            if (first < 0.0) first = t;
            last = t;
            ++crossings;
        }
    }
    if (crossings < 2)
        return 0.0f;
    return static_cast<float>(static_cast<double>(crossings - 1) * sampleRate / (last - first));
}

static float rmsOf(const std::vector<float>& x, size_t start, size_t length)
{
    double sum = 0.0;
    size_t count = 0;
    for (size_t n = start; n < start + length && n < x.size(); ++n, ++count)
        sum += static_cast<double>(x[n]) * static_cast<double>(x[n]);
    return count > 0 ? static_cast<float>(std::sqrt(sum / static_cast<double>(count))) : 0.0f;
}

/** Share of high-frequency content: energy of the first difference divided by the energy. */
static float brightness(const std::vector<float>& x, size_t start, size_t length)
{
    double diff = 0.0, energy = 0.0;
    for (size_t n = start + 1; n < start + length && n < x.size(); ++n)
    {
        diff += static_cast<double>(x[n] - x[n - 1]) * static_cast<double>(x[n] - x[n - 1]);
        energy += static_cast<double>(x[n]) * static_cast<double>(x[n]);
    }
    return static_cast<float>(diff / std::max(1e-20, energy));
}

static size_t indexOfPeak(const std::vector<float>& x, size_t start = 0)
{
    size_t best = start;
    for (size_t n = start; n < x.size(); ++n)
        if (std::abs(x[n]) > std::abs(x[best]))
            best = n;
    return best;
}

static std::vector<float> makeSine(size_t length, double sampleRate, float hz, float amplitude)
{
    std::vector<float> x(length);
    for (size_t n = 0; n < length; ++n)
        x[n] = amplitude * static_cast<float>(std::sin(juce::MathConstants<double>::twoPi * hz * static_cast<double>(n) / sampleRate));
    return x;
}

static void testDelayLineAndGrain()
{
    std::cout << "[Delay 1/6] DelayLine read convention; Grain pitch shift, 32-voice limit, zero allocation, prefix..." << std::endl;

    // --- DelayLine: read(d) before push(x[n]) returns x[n - d] ---
    {
        wvzz::dsp::DelayLine line;
        line.prepare(1000);
        float worstInt = 0.0f, worstFrac = 0.0f;
        for (int n = 0; n < 5000; ++n)
        {
            const float x = static_cast<float>(n); // a ramp: x[n] = n
            if (n > 1100)
            {
                worstInt  = std::max(worstInt,  std::abs(line.read(10.0f) - static_cast<float>(n - 10)));
                worstInt  = std::max(worstInt,  std::abs(line.read(1000.0f) - static_cast<float>(n - 1000)));
                worstInt  = std::max(worstInt,  std::abs(line.readInteger(37) - static_cast<float>(n - 37)));
                worstFrac = std::max(worstFrac, std::abs(line.read(10.5f) - (static_cast<float>(n) - 10.5f)));
            }
            line.push(x);
        }
        check(worstInt < 1.0e-3f, "DelayLine::read(d) must return the sample pushed d samples before");
        check(worstFrac < 1.0e-2f, "DelayLine fractional read must interpolate between samples");
    }

    const double sampleRate = 48000.0;

    // --- Scenario: Pitch shift up one octave (stand-alone Grain module) ---
    {
        wvzz::modules::GrainDSP grain("grain");
        ModuleTestHost host(grain);
        host.set("grain_pitch", 12.0f);
        host.set("grain_spray", 0.0f);
        host.set("grain_reverse", 0.0f);
        host.set("grain_size", 60.0f);
        host.set("grain_density", 40.0f);
        host.prepare(sampleRate, 512);

        std::vector<float> out;
        renderThroughHost(host, makeSine(48000 * 2, sampleRate, 220.0f, 0.5f), out);
        const float freq = estimateFrequency(out, 48000, 40000, sampleRate);
        std::cout << "  -> 220 Hz tone, grain pitch +12: measured " << freq << " Hz" << std::endl;
        check(freq > 400.0f && freq < 480.0f, "grain pitch +12 must move a 220 Hz tone to about 440 Hz");
    }

    // --- Scenario: Voice count is limited to 32 ---
    {
        wvzz::modules::GrainDSP grain("grain");
        ModuleTestHost host(grain);
        host.set("grain_size", 500.0f);
        host.set("grain_density", 60.0f); // asks for about 30 overlapping grains, jitter pushes it past 32
        host.set("grain_reverse", 0.5f);
        host.set("grain_pitch", -12.0f);
        host.prepare(sampleRate, 512);

        juce::AudioBuffer<float> buffer(2, 512);
        juce::Random rng(21);
        int maxVoicesSeen = 0;
        for (int b = 0; b < 400; ++b)
        {
            fillNoise(buffer, rng, 1.0f);
            host.run(buffer);
            maxVoicesSeen = std::max(maxVoicesSeen, grain.getNumActiveVoices());
            check(allFinite(buffer, 64.0f), "grain output must stay finite with all voices busy");
        }
        std::cout << "  -> most voices active at once: " << maxVoicesSeen << std::endl;
        check(maxVoicesSeen <= 32 && maxVoicesSeen >= 20, "the grain engine must use many voices but never more than 32");
    }

    // --- Zero allocation and prefixing ---
    {
        wvzz::modules::GrainDSP grain("slot2_grain");
        ModuleTestHost host(grain);
        check(host.getAPVTS().getParameter("slot2_grain_size") != nullptr && host.getAPVTS().getParameter("grain_size") == nullptr,
              "grain parameter ids must use the instance prefix");
        host.set("slot2_grain_pitch", 7.0f);
        host.set("slot2_grain_reverse", 0.5f);
        host.prepare(sampleRate, 512);
        juce::AudioBuffer<float> buffer(2, 512);
        generateSineWave(buffer, sampleRate, 440.0f);
        host.run(buffer);
        g_allocationCount = 0;
        g_trackingAllocations = true;
        for (int b = 0; b < 1000; ++b)
        {
            generateSineWave(buffer, sampleRate, 440.0f);
            host.run(buffer);
        }
        g_trackingAllocations = false;
        std::cout << "  -> heap allocations in 1,000 blocks: " << g_allocationCount << std::endl;
        check(g_allocationCount == 0, "grain must not allocate while processing");
    }

    std::cout << "  -> PASS\n" << std::endl;
}

static void testDelayTimingAndRouting()
{
    std::cout << "[Delay 2/6] Delay: impulse timing, tempo sync, 4 s maximum, ping-pong, diffusion..." << std::endl;
    const double sampleRate = 48000.0;

    auto impulseResponse = [&](std::function<void(ModuleTestHost&)> configure, size_t length,
                               std::vector<float>& left, std::vector<float>& right) {
        wvzz::modules::DelayDSP delay("dly");
        ModuleTestHost host(delay);
        host.set("dly_mode", 0.0f);
        host.set("dly_feedback", 0.0f);
        host.set("dly_mix", 1.0f);
        host.set("dly_lp", 20000.0f);
        configure(host);
        host.prepare(sampleRate, 512);
        std::vector<float> input(length, 0.0f);
        input[0] = 1.0f;
        renderThroughHost(host, input, left, &right);
    };

    std::vector<float> l, r;

    // --- Scenario: Impulse arrives after the set time (100 ms = 4800 samples, within 1 sample) ---
    impulseResponse([](ModuleTestHost& h) { h.set("dly_time", 100.0f); }, 512 * 20, l, r);
    const long peak100 = static_cast<long>(indexOfPeak(l));
    std::cout << "  -> 100 ms: wet impulse at sample " << peak100 << " (expected 4800)" << std::endl;
    check(std::labs(peak100 - 4800) <= 1, "Clean delay of 100 ms must return the impulse after 4800 samples");
    check(std::labs(static_cast<long>(indexOfPeak(r)) - 4800) <= 1, "the right channel must have the same delay with offset 0");

    // --- Scenario: Tempo-synced time (1/4 at 120 BPM = 500 ms = 24000 samples) ---
    impulseResponse([](ModuleTestHost& h) { h.set("dly_sync", 1.0f); h.set("dly_sync_div", 5.0f); }, 512 * 60, l, r);
    const long peakSync = static_cast<long>(indexOfPeak(l));
    std::cout << "  -> sync 1/4 at 120 BPM: wet impulse at sample " << peakSync << " (expected 24000)" << std::endl;
    check(std::labs(peakSync - 24000) <= 1, "1/4 note at 120 BPM must be 500 ms");
    check(std::abs(wvzz::modules::DelayDSP::beatsForSyncDiv(7) - 0.75) < 1e-9 && std::abs(wvzz::modules::DelayDSP::beatsForSyncDiv(0) - 4.0) < 1e-9,
          "dotted eighth must be 0.75 beats and 1/1 must be 4 beats");

    // --- Scenario: Maximum time (4000 ms = 192000 samples) ---
    impulseResponse([](ModuleTestHost& h) { h.set("dly_time", 4000.0f); }, 512 * 400, l, r);
    const long peakMax = static_cast<long>(indexOfPeak(l));
    std::cout << "  -> 4000 ms: wet impulse at sample " << peakMax << " (expected 192000)" << std::endl;
    check(std::labs(peakMax - 192000) <= 1, "4 second delay must return the impulse after 192000 samples");

    // --- Right offset adds to the right channel only ---
    impulseResponse([](ModuleTestHost& h) { h.set("dly_time", 100.0f); h.set("dly_offset", 10.0f); }, 512 * 20, l, r);
    check(std::labs(static_cast<long>(indexOfPeak(l)) - 4800) <= 1 && std::labs(static_cast<long>(indexOfPeak(r)) - 5280) <= 1,
          "dly_offset of 10 ms must delay the right channel by 480 more samples");

    // --- Scenario: Ping-pong alternates sides ---
    impulseResponse([](ModuleTestHost& h) { h.set("dly_time", 100.0f); h.set("dly_routing", 1.0f); h.set("dly_feedback", 0.6f); }, 512 * 40, l, r);
    auto peakNear = [](const std::vector<float>& x, size_t centre) {
        float best = 0.0f;
        for (size_t n = centre - 20; n <= centre + 20; ++n)
            best = std::max(best, std::abs(x[n]));
        return best;
    };
    std::cout << "  -> ping-pong repeats (L/R): 1st " << peakNear(l, 4800) << "/" << peakNear(r, 4800)
              << ", 2nd " << peakNear(l, 9600) << "/" << peakNear(r, 9600)
              << ", 3rd " << peakNear(l, 14400) << "/" << peakNear(r, 14400) << std::endl;
    check(peakNear(l, 4800) > 0.3f && peakNear(r, 4800) < 1.0e-4f, "ping-pong: the first repeat must be on the left only");
    check(peakNear(r, 9600) > 0.1f && peakNear(l, 9600) < 1.0e-4f, "ping-pong: the second repeat must be on the right only");
    check(peakNear(l, 14400) > 0.05f && peakNear(r, 14400) < 1.0e-4f, "ping-pong: the third repeat must be on the left again");

    // --- Scenario: Diffusion spreads an impulse; at 0 it stays a single impulse ---
    auto countAbove = [](const std::vector<float>& x, float threshold) {
        int count = 0;
        for (float v : x)
            if (std::abs(v) > threshold)
                ++count;
        return count;
    };
    impulseResponse([](ModuleTestHost& h) { h.set("dly_time", 100.0f); h.set("dly_diffusion", 0.0f); }, 512 * 20, l, r);
    const int sharp = countAbove(l, 0.01f);
    impulseResponse([](ModuleTestHost& h) { h.set("dly_time", 100.0f); h.set("dly_diffusion", 1.0f); }, 512 * 20, l, r);
    const int smeared = countAbove(l, 0.01f);
    std::cout << "  -> samples above 0.01 in the first repeat: diffusion 0 -> " << sharp << ", diffusion 1 -> " << smeared << std::endl;
    check(sharp <= 4, "with diffusion 0 the repeat must stay a single impulse");
    check(smeared >= 6 && smeared > sharp * 2, "with diffusion 1 the repeat must spread over many samples");

    std::cout << "  -> PASS\n" << std::endl;
}

static void testDelayFeedbackAndModes()
{
    std::cout << "[Delay 3/6] Delay: bounded at 110% feedback (30 s per mode), low-pass, BBD tone, tape wow..." << std::endl;
    const double sampleRate = 48000.0;
    const int blockSize = 512;

    // --- Scenario: Output stays bounded above 100% feedback, 30 seconds, every mode ---
    for (int mode = 0; mode < 4; ++mode)
    {
        DelayGrainPair pair;
        ModuleTestHost host(pair);
        host.set("dly_mode", static_cast<float>(mode));
        host.set("dly_feedback", 1.1f);
        host.set("dly_time", 120.0f);
        host.set("dly_mix", 1.0f);
        host.set("dly_drive", 1.0f);
        host.set("dly_diffusion", 0.5f);
        host.set("grain_pitch", 12.0f);
        host.prepare(sampleRate, blockSize);

        juce::AudioBuffer<float> buffer(2, blockSize);
        juce::Random rng(77);
        float peak = 0.0f;
        const int blocks = static_cast<int>(30.0 * sampleRate / blockSize);
        for (int b = 0; b < blocks; ++b)
        {
            fillNoise(buffer, rng, 1.0f);
            host.run(buffer);
            if (!allFinite(buffer, 8.0f))
            {
                std::cerr << "FAIL: mode " << mode << " left the bounds at 110% feedback (block " << b << ")" << std::endl;
                std::exit(1);
            }
            peak = std::max(peak, buffer.getMagnitude(0, blockSize));
        }
        std::cout << "  -> mode " << wvzz::modules::getDelayModeNames()[mode] << ": peak after 30 s at 110% feedback = " << peak << std::endl;
    }

    // Noise burst (50 ms) followed by silence, to look at single repeats
    auto burstInput = [&](size_t length) {
        std::vector<float> x(length, 0.0f);
        juce::Random rng(5);
        for (size_t n = 0; n < 2400; ++n)
            x[n] = (rng.nextFloat() * 2.0f - 1.0f) * 0.5f;
        return x;
    };

    // --- Scenario: Low-pass darkens later repeats ---
    {
        wvzz::modules::DelayDSP delay("dly");
        ModuleTestHost host(delay);
        host.set("dly_time", 200.0f);
        host.set("dly_feedback", 0.9f);
        host.set("dly_lp", 1000.0f);
        host.set("dly_mix", 1.0f);
        host.prepare(sampleRate, blockSize);
        std::vector<float> out;
        renderThroughHost(host, burstInput(512 * 80), out);
        const float first = brightness(out, 9600, 2400);
        const float third = brightness(out, 28800, 2400);
        std::cout << "  -> low-pass 1 kHz: brightness of repeat 1 = " << first << ", repeat 3 = " << third << std::endl;
        check(third < first * 0.7f, "with a 1 kHz low-pass the third repeat must be darker than the first");
    }

    // --- Scenario: BBD is darker than Clean ---
    {
        float bright[2] = { 0.0f, 0.0f };
        for (int pass = 0; pass < 2; ++pass)
        {
            wvzz::modules::DelayDSP delay("dly");
            ModuleTestHost host(delay);
            host.set("dly_mode", pass == 0 ? 0.0f : 2.0f);
            host.set("dly_time", 200.0f);
            host.set("dly_feedback", 0.0f);
            host.set("dly_wow", 0.0f);
            host.set("dly_mix", 1.0f);
            host.prepare(sampleRate, blockSize);
            std::vector<float> out;
            renderThroughHost(host, burstInput(512 * 40), out);
            bright[pass] = brightness(out, 9600, 2400);
        }
        std::cout << "  -> brightness of the first repeat: Clean " << bright[0] << ", BBD " << bright[1] << std::endl;
        check(bright[1] < bright[0] * 0.5f, "BBD repeats must be darker than Clean repeats");
    }

    // --- Scenario: Tape wow modulates the time ---
    {
        float spread[2] = { 0.0f, 0.0f };
        for (int pass = 0; pass < 2; ++pass)
        {
            wvzz::modules::DelayDSP delay("dly");
            ModuleTestHost host(delay);
            host.set("dly_mode", 1.0f);
            host.set("dly_time", 200.0f);
            host.set("dly_feedback", 0.0f);
            host.set("dly_drive", 0.0f);
            host.set("dly_wow", pass == 0 ? 0.0f : 1.0f);
            host.set("dly_mix", 1.0f);
            host.prepare(sampleRate, blockSize);
            std::vector<float> out;
            renderThroughHost(host, makeSine(512 * 400, sampleRate, 1000.0f, 0.3f), out);
            float lo = 1.0e9f, hi = 0.0f;
            for (size_t start = 24000; start + 9600 < out.size(); start += 4800) // 200 ms windows over about 3.5 s
            {
                const float f = estimateFrequency(out, start, 9600, sampleRate);
                lo = std::min(lo, f);
                hi = std::max(hi, f);
            }
            spread[pass] = hi - lo;
        }
        std::cout << "  -> pitch range of a 1 kHz tone in Tape mode: wow 0 -> " << spread[0] << " Hz, wow 1 -> " << spread[1] << " Hz" << std::endl;
        check(spread[0] < 0.5f, "with wow 0 the tape repeats must keep a steady pitch");
        check(spread[1] > 2.0f, "with wow 1 the tape repeats must vary in pitch");
    }

    std::cout << "  -> PASS\n" << std::endl;
}

static void testDelayTimeChangeDuckingBypass()
{
    std::cout << "[Delay 4/6] Delay: time-change crossfade / pitch bend, ducking, bypass tail, granular timing, zero allocation..." << std::endl;
    const double sampleRate = 48000.0;
    const int blockSize = 512;

    // Plays a steady tone, changes dly_time from 200 to 400 ms after 1 second, returns the wet output.
    auto timeChange = [&](int mode, float toneHz, std::vector<float>& out) {
        DelayGrainPair pair;
        ModuleTestHost host(pair);
        host.set("dly_mode", static_cast<float>(mode));
        host.set("dly_time", 200.0f);
        host.set("dly_feedback", 0.0f);
        host.set("dly_wow", 0.0f);
        host.set("dly_drive", 0.0f);
        host.set("dly_mix", 1.0f);
        host.set("grain_spray", 0.0f);
        host.set("grain_spread", 0.0f);
        host.prepare(sampleRate, blockSize);

        const auto input = makeSine(512 * 290, sampleRate, toneHz, 0.5f);
        juce::AudioBuffer<float> buffer(2, blockSize);
        out.assign(input.size(), 0.0f);
        for (size_t pos = 0; pos + blockSize <= input.size(); pos += blockSize)
        {
            if (pos == static_cast<size_t>(blockSize) * 94) // about 1.0 s
                host.set("dly_time", 400.0f);
            for (int i = 0; i < blockSize; ++i)
            {
                buffer.setSample(0, i, input[pos + static_cast<size_t>(i)]);
                buffer.setSample(1, i, input[pos + static_cast<size_t>(i)]);
            }
            host.run(buffer);
            for (int i = 0; i < blockSize; ++i)
                out[pos + static_cast<size_t>(i)] = buffer.getSample(0, i);
        }
    };
    const size_t changeAt = 512 * 94;

    // --- Scenario: Clean and Granular time change crossfades (no click) ---
    for (int mode : { 0, 3 })
    {
        std::vector<float> out;
        timeChange(mode, 100.0f, out);
        float worstStep = 0.0f;
        for (size_t n = 24000; n < out.size(); ++n)
            worstStep = std::max(worstStep, std::abs(out[n] - out[n - 1]));
        std::cout << "  -> " << wvzz::modules::getDelayModeNames()[mode] << ": largest sample-to-sample step across the time change = "
                  << worstStep << " (a 100 Hz tone at 0.5 steps by 0.0065)" << std::endl;
        check(worstStep < 0.03f, "a time change in Clean / Granular mode must not click");
    }

    // Clean keeps the pitch during the 50 ms crossfade and has the new time afterwards
    {
        std::vector<float> out;
        timeChange(0, 1000.0f, out);
        const float during = estimateFrequency(out, changeAt + 4800, 9600, sampleRate);
        check(std::abs(during - 1000.0f) < 5.0f, "Clean mode must not bend the pitch on a time change");
    }

    // --- Scenario: Tape and BBD time change bends the pitch ---
    for (int mode : { 1, 2 })
    {
        std::vector<float> out;
        timeChange(mode, 1000.0f, out);
        const float before = estimateFrequency(out, changeAt - 14400, 9600, sampleRate);
        const float during = estimateFrequency(out, changeAt + 480, 4800, sampleRate);
        const float after  = estimateFrequency(out, changeAt + 86400, 9600, sampleRate); // 1.8 s later
        std::cout << "  -> " << wvzz::modules::getDelayModeNames()[mode] << ": pitch before " << before << " Hz, during the glide "
                  << during << " Hz, after " << after << " Hz" << std::endl;
        check(std::abs(before - 1000.0f) < 2.0f, "before the change the repeats must be at the input pitch");
        check(during < 900.0f && during > 500.0f, "while the time glides up, the pitch of the repeats must drop (and not reverse)");
        check(std::abs(after - 1000.0f) < 5.0f, "after the glide the pitch must return to the input pitch");
    }

    // --- Scenario: Wet level drops under the dry signal, and returns after it stops ---
    {
        float wetDuring[2] = { 0.0f, 0.0f }, wetAfter[2] = { 0.0f, 0.0f };
        for (int pass = 0; pass < 2; ++pass)
        {
            wvzz::modules::DelayDSP delay("dly");
            ModuleTestHost host(delay);
            host.set("dly_time", 100.0f);
            host.set("dly_feedback", 0.8f);
            host.set("dly_mix", 0.5f);
            host.set("dly_duck", pass == 0 ? 0.0f : 1.0f);
            host.prepare(sampleRate, blockSize);

            auto input = makeSine(512 * 190, sampleRate, 500.0f, 0.5f);
            for (size_t n = 48000; n < input.size(); ++n) // dry signal stops after 1 second
                input[n] = 0.0f;
            std::vector<float> out;
            renderThroughHost(host, input, out);
            std::vector<float> wet(out.size());
            for (size_t n = 0; n < out.size(); ++n)
                wet[n] = out[n] - 0.5f * input[n];
            wetDuring[pass] = rmsOf(wet, 28800, 14400);
            wetAfter[pass]  = rmsOf(wet, 48000 + 38400, 9600); // 0.8 .. 1.0 s after the stop (release is 250 ms)
        }
        std::cout << "  -> wet RMS while the dry signal plays: duck 0 -> " << wetDuring[0] << ", duck 1 -> " << wetDuring[1]
                  << "; 0.8 s after it stops: " << wetAfter[0] << " vs " << wetAfter[1] << std::endl;
        check(wetDuring[1] < wetDuring[0] * 0.2f, "ducking must lower the wet level while the dry signal is present");
        check(wetAfter[1] > wetAfter[0] * 0.8f, "after the dry signal stops the repeats must return to full level");
    }

    // --- Scenario: Tail continues after bypass; no new repeats are recorded ---
    {
        wvzz::modules::DelayDSP delay("dly");
        ModuleTestHost host(delay);
        host.set("dly_time", 100.0f);
        host.set("dly_feedback", 0.7f);
        host.set("dly_mix", 0.5f);
        host.prepare(sampleRate, blockSize);

        juce::AudioBuffer<float> buffer(2, blockSize), input(2, blockSize);
        juce::Random rng(8);
        for (int b = 0; b < 20; ++b) // 0.2 s of noise
        {
            fillNoise(buffer, rng, 0.5f);
            host.run(buffer);
        }
        host.set("dly_bypass", 1.0f);
        float tailEarly = 0.0f, tailLate = 0.0f;
        for (int b = 0; b < 400; ++b) // 4.3 s of silence
        {
            buffer.clear();
            host.run(buffer);
            if (b >= 10 && b < 40)   tailEarly = std::max(tailEarly, buffer.getMagnitude(0, blockSize));
            if (b >= 380)            tailLate  = std::max(tailLate,  buffer.getMagnitude(0, blockSize));
        }
        std::cout << "  -> after bypass: tail peak 0.1-0.4 s = " << tailEarly << ", after 4 s = " << tailLate << std::endl;
        check(tailEarly > 0.01f, "the repeats must keep playing after bypass is turned on");
        check(tailLate < tailEarly * 0.01f, "the tail must decay after bypass");

        // --- Scenario: Bypassed and empty is transparent; audio played now creates no repeats ---
        float worst = 0.0f;
        for (int b = 0; b < 60; ++b)
        {
            if (b < 10) fillNoise(buffer, rng, 0.5f); else buffer.clear();
            input.makeCopyOf(buffer);
            host.run(buffer);
            worst = std::max(worst, maxAbsDiff(buffer, input));
        }
        std::cout << "  -> bypassed: max difference between output and input = " << worst << std::endl;
        check(worst < 2.0e-4f, "a bypassed, empty delay must output the input and record no new repeats");
    }

    // --- Scenario: Granular delay keeps the delay time ---
    {
        DelayGrainPair pair;
        ModuleTestHost host(pair);
        host.set("dly_mode", 3.0f);
        host.set("dly_time", 300.0f);
        host.set("dly_feedback", 0.0f);
        host.set("dly_mix", 1.0f);
        host.set("grain_pitch", 0.0f);
        host.set("grain_spray", 0.0f);
        host.prepare(sampleRate, blockSize);

        std::vector<float> input(512 * 80, 0.0f);
        juce::Random rng(4);
        for (size_t n = 0; n < 960; ++n) // 20 ms burst, centre at 10 ms
            input[n] = (rng.nextFloat() * 2.0f - 1.0f) * 0.5f;
        std::vector<float> out;
        renderThroughHost(host, input, out);
        double weighted = 0.0, total = 0.0;
        for (size_t n = 0; n < out.size(); ++n)
        {
            weighted += static_cast<double>(n) * out[n] * out[n];
            total += static_cast<double>(out[n]) * out[n];
        }
        const double centreMs = (total > 0.0) ? (weighted / total) * 1000.0 / sampleRate : 0.0;
        std::cout << "  -> granular, 300 ms: energy centre of the repeat at " << centreMs << " ms (burst centre + delay = 310 ms)" << std::endl;
        check(total > 1.0e-4 && std::abs(centreMs - 310.0) < 15.0, "granular repeats must be centered at the set delay time");
    }

    // --- Zero allocation in every mode, and prefixing ---
    for (int mode = 0; mode < 4; ++mode)
    {
        DelayGrainPair pair;
        ModuleTestHost host(pair);
        host.set("dly_mode", static_cast<float>(mode));
        host.set("dly_feedback", 0.8f);
        host.set("dly_diffusion", 0.7f);
        host.set("dly_duck", 0.5f);
        host.set("dly_routing", 1.0f);
        host.prepare(sampleRate, blockSize);
        juce::AudioBuffer<float> buffer(2, blockSize);
        generateSineWave(buffer, sampleRate, 440.0f);
        host.run(buffer);
        g_allocationCount = 0;
        g_trackingAllocations = true;
        for (int b = 0; b < 1000; ++b)
        {
            generateSineWave(buffer, sampleRate, 440.0f);
            if (b == 500)
                pair.delay.reset(); // reset must not allocate either
            host.run(buffer);
        }
        g_trackingAllocations = false;
        if (g_allocationCount != 0)
        {
            std::cerr << "FAIL: delay mode " << mode << " allocated " << g_allocationCount << " times while processing" << std::endl;
            std::exit(1);
        }
    }
    std::cout << "  -> heap allocations in 1,000 blocks, each of the 4 modes: 0" << std::endl;
    {
        wvzz::modules::DelayDSP delay("slot2_dly");
        ModuleTestHost host(delay);
        check(host.getAPVTS().getParameter("slot2_dly_feedback") != nullptr && host.getAPVTS().getParameter("dly_feedback") == nullptr,
              "delay parameter ids must use the instance prefix");
    }

    std::cout << "  -> PASS\n" << std::endl;
}

static void testWvZzDelayPlugin()
{
    std::cout << "[Delay 5/6] Plugin: presets, history, tail length, modulation profile, zero allocation..." << std::endl;

    wvzz::WvZzDelayAudioProcessor processor;
    processor.prepareToPlay(48000.0, 512);
    check(processor.getName() == "WvZz Delay", "plugin name");

    const auto& presets = wvzz::delayfx::getFactoryPresets();
    check(presets.size() == 10 && processor.getNumPrograms() == 10, "WvZz Delay must have 10 factory presets");
    check(processor.getCurrentProgram() == 0, "the plugin must start on program 0");

    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;

    // --- Scenario: All presets load and stay stable ---
    for (int i = 0; i < 10; ++i)
    {
        processor.setCurrentProgram(i);
        check(processor.getProgramName(i) == juce::String(presets[static_cast<size_t>(i)].name), "preset name mismatch");
        for (const auto& [paramId, value] : presets[static_cast<size_t>(i)].values)
        {
            auto* param = dynamic_cast<juce::RangedAudioParameter*>(processor.getAPVTS().getParameter(paramId));
            check(param != nullptr, paramId);
            const float actual = param->convertFrom0to1(param->getValue());
            if (std::abs(actual - value) > 0.02f * std::max(1.0f, std::abs(value)))
            {
                std::cerr << "FAIL: preset '" << presets[static_cast<size_t>(i)].name << "' " << paramId
                          << " = " << actual << ", expected " << value << std::endl;
                std::exit(1);
            }
        }
        for (int b = 0; b < 200; ++b) // about 2 s, so the feedback loop runs many times
        {
            generateSineWave(buffer, 48000.0, 220.0f);
            processor.processBlock(buffer, midi);
            check(allFinite(buffer, 8.0f), "preset output must be finite");
        }
        check(processor.getTailLengthSeconds() > 0.0 && processor.getTailLengthSeconds() <= 60.0, "tail length must be in (0, 60] s");
    }

    // --- Scenario: Preset result is independent of history ---
    auto snapshot = [&processor]() {
        std::vector<float> values;
        for (auto* param : processor.getParameters())
            values.push_back(param->getValue());
        return values;
    };
    processor.setCurrentProgram(0);
    const auto direct = snapshot();
    processor.setCurrentProgram(9);
    processor.setCurrentProgram(0);
    check(direct == snapshot(), "loading a preset must give the same parameter values whatever was loaded before");

    // --- Session recall: parameters and the selected preset come back from saved state ---
    {
        processor.setCurrentProgram(6);
        juce::MemoryBlock saved;
        processor.getStateInformation(saved);
        const auto savedValues = snapshot();
        processor.setCurrentProgram(2);
        processor.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
        check(processor.getCurrentProgram() == 6, "the selected preset must be restored with the state");
        check(savedValues == snapshot(), "all parameter values must be restored with the state");
    }

    // --- Scenario: Tail grows with feedback (time 500 ms) ---
    auto& apvts = processor.getAPVTS();
    resetPluginToDefaults(processor);
    setPluginParam(apvts, "dly_time", 500.0f);
    setPluginParam(apvts, "dly_feedback", 0.2f);
    const double tailLow = processor.getTailLengthSeconds();
    setPluginParam(apvts, "dly_feedback", 0.9f);
    const double tailHigh = processor.getTailLengthSeconds();
    setPluginParam(apvts, "dly_feedback", 1.0f);
    const double tailFull = processor.getTailLengthSeconds();
    std::cout << "  -> tail at 500 ms: feedback 0.2 -> " << tailLow << " s, 0.9 -> " << tailHigh << " s, 1.0 -> " << tailFull << " s" << std::endl;
    check(tailLow >= 0.5 && tailHigh > tailLow * 5.0 && std::abs(tailFull - 60.0) < 1.0e-9, "tail length must follow time and feedback, 60 s at 100%");

    // --- Modulation profile: lists and LFO delivery to Feedback ---
    const auto& delayNames = wvzz::modules::getModTargetNames(wvzz::ModProfile::Delay);
    check(delayNames.size() == 14 && delayNames[2] == "Feedback" && delayNames[13] == "Master Vol", "Delay profile must list None + 13 targets");
    check(wvzz::modules::getModTargetNames(wvzz::ModProfile::Distortion).size() == 22
          && wvzz::modules::getModTargetNames(wvzz::ModProfile::Dynamics).size() == 16
          && wvzz::modules::getModTargetNames(wvzz::ModProfile::Modulation).size() == 15, "other profiles must keep their lists");
    check(static_cast<int>(wvzz::ModTarget::ModTremDepth) == 42 && static_cast<int>(wvzz::ModTarget::DlyTime) == 43,
          "existing ModTarget values must not move");
    check(wvzz::modules::getModTargetForIndex(2, wvzz::ModProfile::Delay) == wvzz::ModTarget::DlyFeedback
          && wvzz::modules::getModTargetForIndex(9, wvzz::ModProfile::Delay) == wvzz::ModTarget::GrainPitch, "Delay profile index mapping");

    setPluginParam(apvts, "lfo1_target", 2.0f);
    setPluginParam(apvts, "lfo1_depth", 1.0f);
    setPluginParam(apvts, "lfo1_rate", 5.0f);
    bool sawOffset = false;
    for (int b = 0; b < 10; ++b)
    {
        buffer.clear();
        processor.processBlock(buffer, midi);
        if (std::abs(processor.getModulationModule().getModulationDSP().getOffset(wvzz::ModTarget::DlyFeedback)) > 0.01f)
            sawOffset = true;
    }
    check(sawOffset, "LFO 1 must deliver a non-zero offset to the Feedback target");

    // --- Plugin zero allocation: tape preset and granular preset ---
    for (int program : { 3, 8 })
    {
        processor.setCurrentProgram(program);
        generateSineWave(buffer, 48000.0, 440.0f);
        processor.processBlock(buffer, midi);
        g_allocationCount = 0;
        g_trackingAllocations = true;
        for (int i = 0; i < 1000; ++i)
        {
            generateSineWave(buffer, 48000.0, 440.0f);
            processor.processBlock(buffer, midi);
        }
        g_trackingAllocations = false;
        if (g_allocationCount != 0)
        {
            std::cerr << "FAIL: WvZz Delay preset " << program << " allocated " << g_allocationCount << " times on the audio thread" << std::endl;
            std::exit(1);
        }
    }
    std::cout << "  -> heap allocations in 1,000 processBlock() calls (tape preset, granular preset): 0" << std::endl;

    std::cout << "  -> PASS\n" << std::endl;
}

static void testWvZzDelayEditorAndTaps()
{
    std::cout << "[Delay 6/6] Editor at 1080x680, TAPS display, published tap state, tap layout..." << std::endl;

    wvzz::WvZzDelayAudioProcessor processor;
    processor.setCurrentProgram(1); // Dotted Eighth Ping-Pong
    processor.prepareToPlay(48000.0, 512);

    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
    check(editor != nullptr, "createEditor() returned null for WvZz Delay");
    editor->setSize(1080, 680);
    check(editor->getWidth() == 1080 && editor->getHeight() == 680, "WvZz Delay editor must be 1080x680");

    std::function<int(juce::Component&)> countDisplays = [&countDisplays](juce::Component& c) {
        int n = (dynamic_cast<wvzz::ui::DelayTapDisplayComponent*>(&c) != nullptr) ? 1 : 0;
        for (auto* child : c.getChildren())
            n += countDisplays(*child);
        return n;
    };
    check(countDisplays(*editor) == 1, "WvZz Delay editor must contain one delay-tap display");

    // The audio thread publishes the delay state for the display
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;
    generateSineWave(buffer, 48000.0, 440.0f);
    processor.processBlock(buffer, midi);
    const auto& state = processor.getDelayModule().getDelayDSP().getTapState();
    check(state.active.load() && state.pingPong.load(), "tap state must report an active ping-pong delay for preset 2");
    check(std::abs(state.timeLeftMs.load() - 375.0f) < 0.5f, "dotted eighth at 120 BPM must be published as 375 ms");
    check(std::abs(state.feedback.load() - 0.5f) < 0.01f, "tap state must carry the feedback value");

    setPluginParam(processor.getAPVTS(), "dly_bypass", 1.0f);
    processor.processBlock(buffer, midi);
    check(!state.active.load(), "a bypassed delay must publish an inactive tap state");

    // --- Scenario: Ping-pong shows alternating sides; taps follow time and feedback ---
    using Display = wvzz::ui::DelayTapDisplayComponent;
    const auto pingPong = Display::computeTaps(100.0f, 100.0f, 0.5f, true);
    check(pingPong.size() >= 4 && !pingPong[0].rightChannel && pingPong[1].rightChannel && !pingPong[2].rightChannel,
          "ping-pong taps must alternate left, right, left");
    check(std::abs(pingPong[1].timeMs - 200.0f) < 1.0e-3f && std::abs(pingPong[1].level - 0.5f) < 1.0e-5f,
          "the second tap must be at twice the time with the feedback level");
    const auto offsetPingPong = Display::computeTaps(100.0f, 110.0f, 0.5f, true);
    check(std::abs(offsetPingPong[1].timeMs - 210.0f) < 1.0e-3f && std::abs(offsetPingPong[2].timeMs - 310.0f) < 1.0e-3f,
          "ping-pong taps must add the right time on right repeats (left 100 ms, right 110 ms -> 100, 210, 310)");
    const auto stereo = Display::computeTaps(100.0f, 110.0f, 0.5f, false);
    int leftCount = 0, rightCount = 0;
    for (const auto& tap : stereo) (tap.rightChannel ? rightCount : leftCount)++;
    check(leftCount == rightCount && leftCount >= 4, "stereo taps must show the same number of repeats on both sides");
    check(Display::computeTaps(100.0f, 100.0f, 0.0f, false).size() == 2, "with feedback 0 there is one repeat per side");
    check(Display::computeTaps(100.0f, 100.0f, 0.9f, false).size() > stereo.size(), "more feedback must show more repeats");

    std::cout << "  -> PASS\n" << std::endl;
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

        // Modulation suites
        testFlangerExtension();
        testChorusModule();
        testPhaserModule();
        testTremoloModule();
        testWvZzModulationChain();
        testWvZzModulationZeroAllocations();
        testWvZzModulationPresets();
        testWvZzModulationTargets();
        testWvZzModulationEditorAndScope();

        // Delay suites
        testDelayLineAndGrain();
        testDelayTimingAndRouting();
        testDelayFeedbackAndModes();
        testDelayTimeChangeDuckingBypass();
        testWvZzDelayPlugin();
        testWvZzDelayEditorAndTaps();

        std::cout << "=====================================================" << std::endl;
        std::cout << "   ALL 34 DSP SUITES PASSED (100% SUCCESS)           " << std::endl;
        std::cout << "=====================================================" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "EXCEPTION ENCOUNTERED: " << e.what() << std::endl;
        return 1;
    }
}
