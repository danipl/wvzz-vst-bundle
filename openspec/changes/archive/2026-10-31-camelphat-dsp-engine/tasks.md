# Tasks

## 1. Build System & JUCE 8 Scaffolding

- [x] 1.1 Create `CMakeLists.txt` fetching JUCE 8.0.4 via `FetchContent`, configuring C++20, Universal Binary (`arm64;x86_64`), macOS deployment target 11.0, plugin metadata (`CamA`/`Cph3`), VST3/AU/Standalone targets, and verify `cmake -B build` succeeds.
- [x] 1.2 Implement `Source/PluginProcessor.h` and `Source/PluginProcessor.cpp` baseline scaffolding and verify compilation produces valid VST3, AU, and Standalone bundles in `build/`.

## 2. APVTS Parameter Architecture & Real-Time Safety

- [x] 2.1 Implement `Source/Parameters.h` declaring all APVTS parameter IDs, ranges, default values, and cached atomic float pointers for all 6 modules and master controls.
- [x] 2.2 Wire APVTS into `PluginProcessor`, pre-allocate `globalDryBuffer` and `bandRejectBuffer` in `prepareToPlay`, and verify zero allocations occur during buffer processing.

## 3. Core DSP Modules Implementation

- [x] 3.1 Implement `Source/DSP/BPFilter.h` (dual resonant TPT/SVF splitter separating passband and band-reject feeds) and verify clean frequency separation.
- [x] 3.2 Implement `Source/DSP/DistortionModule.h` containing Tube, Mech, Bit Crusher, and Xcita circuits inside 4x minimum-phase polyphase IIR oversampling with 10 Hz DC blocking.
- [x] 3.3 Implement `Source/DSP/EnvelopeFollower.h` and `Source/DSP/MMFilter.h` (Multi-Mode Lowpass, Bandpass, Highpass filter with dynamic envelope modulation).
- [x] 3.4 Implement `Source/DSP/FlangerModule.h` (stereo modulated delay line, quadrature LFO, and soft-saturating $\tanh$ feedback).
- [x] 3.5 Implement `Source/DSP/MagicEQ.h` (resonant sub-bass boost with saturation) and `Source/DSP/CompressorModule.h` (optical program-dependent compressor with "Phat" mode).
- [x] 3.6 Implement `Source/DSP/MasterSection.h` (dry/wet crossfader, master gain, atomic VU peak/RMS level calculation, and NaN/Inf sanitization clamp).

## 4. Engine Assembly & Interim UI

- [x] 4.1 Assemble the full DSP pipeline in `PluginProcessor::processBlock` with `juce::ScopedNoDenormals` and parameter smoothing using `juce::SmoothedValue`.
- [x] 4.2 Attach `juce::GenericAudioProcessorEditor` in `createEditor()` and verify all parameters are accessible and automatable.

## 5. Verification Harness & Validation

- [x] 5.1 Implement `Tests/DSPTests.cpp` and standalone test target `camelphat_dsp_tests` asserting zero heap allocations in `processBlock()`, harmonic distortion generation, filter stability under extreme resonance, and parameter smoothing.
- [x] 5.2 Run `ctest` / `./build/camelphat_dsp_tests` and run Apple Audio Unit validation (`auval -v aufx Cph3 CamA`) to verify clean validation.

## Workflow follow-up
- Archive the change after Stage 1 verification is complete.
- Kick off Stage 2 (`camelphat-custom-gui`) for custom LookAndFeel, X/Y pads, animated VU meters, and factory presets.
