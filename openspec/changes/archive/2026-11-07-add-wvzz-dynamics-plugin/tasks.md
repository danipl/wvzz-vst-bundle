# Tasks

## 1. Core Interfaces, Modulation Profile, and Shared Module Upgrades

- [x] 1.1 Extend `Source/Core/Interfaces/ProcessContext.h` with Dynamics `ModTarget` entries (`DynSubXover` through `DynClipDrive`) and `cumulativeGainReductionDb` telemetry, and update `ModulationDSP` / `ModulationModule` / `ModulationView` (`Source/Modules/Modulation/`) to support `ModProfile::Distortion` and `ModProfile::Dynamics` target lists without breaking existing Distortion modulation tests
- [x] 1.2 Upgrade `Source/Modules/Compressor/CompressorDSP.{h,cpp}` and `CompressorView.{h,cpp}` with selectable circuit topologies (`0: Opto`, `1: VCA`, `2: FET`) and adjustable `comp_knee` (`0.0` to `12.0 dB`), and verify both legacy Opto tests and new VCA/FET unit tests pass in `wvzz_dsp_tests`
- [x] 1.3 Extend `Source/Core/DSP/AudioVisualizerSource.h` and `Source/Core/UI/SignalComparisonComponent.{h,cpp}` to capture real-time gain reduction samples, render the `GR` mode scrolling trace (`0` to `-24 dB`), and display the live output Crest Factor (dB) readout, and register `?` help entries for all Dynamics modules in `Source/Core/UI/HelpContentRepository.h`

## 2. New Dynamics DSP & UI Modules in `WvZzModules`

- [x] 2.1 Implement `DynDetectorModule` (`Source/Modules/DynDetector/DynDetector{DSP,View,Module}.{h,cpp}`) with 24 dB/oct Linkwitz-Riley sub isolation (`det_sub_xover`, `det_sub_lock`, `det_sub_tight`) and sidechain detector filtering (`det_sc_hp`, `det_sc_tilt`, `det_sc_listen`), and add unit tests verifying phase-coherent sub recombination and sidechain filtering
- [x] 2.2 Implement `TransientShaperModule` (`Source/Modules/TransientShaper/TransientShaper{DSP,View,Module}.{h,cpp}`) with dual-envelope differential attack/sustain shaping (`trans_attack`, `trans_sustain`, `trans_speed`) and hysteresis noise gate (`trans_gate`), and add unit tests verifying transient boost and inter-note gating
- [x] 2.3 Implement `MultibandOTTModule` (`Source/Modules/MultibandOTT/MultibandOTT{DSP,View,Module}.{h,cpp}`) with 3-band (`120 Hz` / `2.5 kHz`) phase-aligned upward/downward compression (`ott_depth`, `ott_up_drive`, `ott_down_drive`, `ott_time`, `ott_high_sheen`), and add unit tests verifying low-level upward boost and high-level downward clamping
- [x] 2.4 Implement `RhythmicPumperModule` (`Source/Modules/RhythmicPumper/RhythmicPumper{DSP,View,Module}.{h,cpp}`) supporting Tempo-Synced PPQ curves (`Classic Duck`, `Hard Kick Clear`, `Reverse Bass`, `Festival Swell`), External/Audio Sidechain triggering, and `<180 Hz` sub-only ducking (`pump_sub_only`), and add unit tests verifying both PPQ sync and audio-triggered ducking
- [x] 2.5 Implement `MaximizerClipperModule` (`Source/Modules/MaximizerClipper/MaximizerClipper{DSP,View,Module}.{h,cpp}`) with input drive (`clip_drive`), brickwall ceiling (`clip_ceiling`), and 1st-order ADAA `Hard Clip`, `Warm Tube`, and `Hardstyle Crunch` transfer curves, and add unit tests verifying ceiling clamping and RMS maximization

## 3. `WvZzDynamics` Plugin Target, Editor, and Factory Presets

- [x] 3.1 Create `Plugins/WvZzDynamics/Presets/FactoryPresets.h`, `Plugins/WvZzDynamics/PluginProcessor.{h,cpp}`, and `Plugins/WvZzDynamics/PluginEditor.{h,cpp}` implementing the full 7-stage zero-allocation `WvZzDynamicsAudioProcessor` (`PLUGIN_CODE WzDy`), stereo sidechain bus support, 20 Hardstyle/Euphoric/Big Room factory presets, and 3-column dark obsidian UI layout
- [x] 3.2 Register all new `WvZzModules` sources and the `WvZzDynamics` plugin target (`VST3` and `Standalone` with macOS ad-hoc codesigning) in `CMakeLists.txt` and update `Makefile` helper targets, verifying `make build` compiles both `WvZz Distortion` and `WvZz Dynamics` cleanly

## 4. End-to-End Verification & Packaging Gate

- [x] 4.1 Expand `Tests/DSPTests.cpp` with comprehensive zero-allocation processor tests, multi-instance prefixing tests, and 20-preset state recall tests for `WvZzDynamicsAudioProcessor`, and verify `make test`, `make package`, and `openspec validate --all` pass with zero errors

## Workflow follow-up

- Sync and archive the `add-wvzz-dynamics-plugin` OpenSpec change (`openspec archive add-wvzz-dynamics-plugin`) before opening or merging the pull request to `main`.
