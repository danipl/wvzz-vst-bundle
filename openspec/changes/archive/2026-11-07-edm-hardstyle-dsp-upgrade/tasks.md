# Tasks

## 1. ProcessContext & Beat-Synced Modulation Engine

- [x] 1.1 Extend `ModTarget` enum (`ModTarget::MasterVol`) and `ProcessContext` struct (`hostBpm`, `hostPpqPosition`, `isHostPlaying`, `distortionLatencySamples`) in `Source/Core/Interfaces/ProcessContext.h`, and verify compilation with `make build`.
- [x] 1.2 Update `ModulationDSP` (`Source/Modules/Modulation/ModulationDSP.h` and `ModulationDSP.cpp`) to support 22 targets (`numTargets = 22`, `"Master Vol"`), read live `hostBpm`, `hostPpqPosition`, and `isHostPlaying` from `ProcessContext`, and lock synced LFO phase to `hostPpqPosition` when playing; add unit test in `Tests/DSPTests.cpp` verifying 150 BPM sync rate and `ModTarget::MasterVol` offset delivery via `make test`.
- [x] 1.3 Update `WvZzDistortionAudioProcessor::processBlock` in `Plugins/WvZzDistortion/PluginProcessor.cpp` to query `bpm`, `ppqPosition`, and `isPlaying` from `getPlayHead()` and pass sub-block-accurate transport values into `ProcessContext`, verifying zero heap allocations via `make test`.

## 2. Phase-Coherent Band-Pass Splitter & Oversampling Latency Alignment

- [x] 2.1 Update `BPFilterDSP` (`Source/Modules/BPFilter/BPFilterDSP.h` and `BPFilterDSP.cpp`) to extract `lowRejected + highRejected` using phase-coherent complementary subtraction and pre-allocate a stereo delay line (`rejDelayBufferL/R`) to delay `auxBandRejectBuffer` by `ctx.distortionLatencySamples` inside `recombine()`; verify phase-coherent recombination in `Tests/DSPTests.cpp` via `make test`.

## 3. Serial Cascaded Distortion with Hard Clip & Wavefold Modes

- [x] 3.1 Add `dist_mode` (`0: Classic`, `1: Hard Clip`, `2: Wavefold`) parameter to `DistortionDSP` (`Source/Modules/Distortion/DistortionDSP.h` and `DistortionDSP.cpp`), implement serial (cascaded) wet/dry saturation across `Mech`, `Tube`, and `Xcita` with `Hard Clip` and `Wavefold` transfer curves, and set `ctx.distortionLatencySamples` to `0` when bypassed/inactive or `getLatencyInSamples()` when active; add unit tests in `Tests/DSPTests.cpp` verifying serial saturation, `Hard Clip`, `Wavefold`, and zero-drive latency alignment via `make test`.
- [x] 3.2 Add the `dist_mode` selector (`Classic / Clip / Fold`) to `DistortionView` (`Source/Modules/Distortion/DistortionView.h` and `DistortionView.cpp`) and update help copy in `Source/Core/UI/HelpContentRepository.h`, verifying non-overlapping layout bounds in `Tests/DSPTests.cpp` via `make test`.

## 4. Pre/Post Distortion Magic EQ Routing for Hardstyle Kicks

- [x] 4.1 Expand `meq_tune` to `45.0–180.0 Hz`, add `meq_pre` (`Magic EQ Pre-Distortion`, default `false`) to `MagicEQDSP` (`Source/Modules/MagicEQ/MagicEQDSP.h` and `MagicEQDSP.cpp`), and route `meqModule` before `distModule` in `Plugins/WvZzDistortion/PluginProcessor.cpp` when `meq_pre` is enabled; add unit test in `Tests/DSPTests.cpp` verifying pre-distortion vs. post-flanger harmonic excitation via `make test`.
- [x] 4.2 Add the `PRE` pill toggle alongside `PHAT` in `MagicEQView` (`Source/Modules/MagicEQ/MagicEQView.h` and `MagicEQView.cpp`) and dynamically update `signalFlowLabel` in `Plugins/WvZzDistortion/PluginEditor.cpp` when `meq_pre` changes, verifying GUI bounds via `make test`.

## 5. Master Sub-Bass Mono Maker, 0 dBFS Clipper & Click-Free Bypass

- [x] 5.1 Update `MasterDSP` (`Source/Modules/Master/MasterDSP.h` and `MasterDSP.cpp`) to read `ctx.distortionLatencySamples`, apply `ModTarget::MasterVol` modulation, smooth `master_bypass` with a click-free crossfade ramp, add `< 120 Hz` Sub-Bass Mono summing (`master_mono_bass`), and add a `0.0 dBFS` Output Clipper (`master_clip`, default `true`); add unit tests in `Tests/DSPTests.cpp` verifying `< 120 Hz` mono summing, `0.0 dBFS` peak ceiling, and click-free bypass via `make test`.
- [x] 5.2 Add `MONO` and `CLIP` pill toggles to `MasterView` (`Source/Modules/Master/MasterView.h` and `MasterView.cpp`) and update `Source/Core/UI/HelpContentRepository.h`, verifying GUI layout via `make test`.

## 6. EDM, Big Room & Hardstyle Factory Presets and Final Verification

- [x] 6.1 Add 5 new presets (`Hardstyle Kick Tok & Crunch`, `Euphoric Supersaw Excite`, `Raw Screech Comb Screamer`, `Reverse Bass Pump`, and `Big Room Festival Kick`) to `Plugins/WvZzDistortion/Presets/FactoryPresets.h`, update `Plugins/WvZzDistortion/README.md` and `USER_MANUAL.md`, and verify all 15 presets recall cleanly in `Tests/DSPTests.cpp` via `make build && make test && openspec validate --all`.

## Workflow follow-up

- Archive the change (`openspec archive edm-hardstyle-dsp-upgrade`) after implementation and verification complete.
