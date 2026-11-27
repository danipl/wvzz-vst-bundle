# Proposal

## Why

Electronic music producers in Hardstyle, Euphoric Hardstyle, Rawstyle, Big Room, and Festival EDM typically stack four to six separate plugins on kick, lead, and master buses (sub crossover, transient shaper, multiband upward/downward OTT compressor, bus glue compressor, tempo-synced sidechain pumper, and anti-aliased brickwall clipper). Adding **WvZz Dynamics** (`PLUGIN_CODE WzDy`) as the second flagship plugin in the `WvZzBundle` suite solves this by unifying all six stages into a zero-allocation, phase-coherent, latency-compensated processor that shares and upgrades the `WvZzModules` library.

## What Changes

- **New Flagship Plugin Target (`WvZzDynamics`)**:
  - Adds `WvZzDynamics` (`PRODUCT_NAME "WvZz Dynamics"`, `PLUGIN_CODE WzDy`) in `Plugins/WvZzDynamics/` targeting macOS Universal Binary (`arm64;x86_64`) VST3 and Standalone formats with an optional stereo external sidechain input bus.
  - Provides 20 genre-crafted factory presets tailored for Hardstyle, Euphoric, Rawstyle, Big Room, and Festival EDM production.
- **5 New Reusable DSP & UI Modules in `WvZzModules`**:
  - `DynDetectorModule` (`det_*`): Linkwitz-Riley 24 dB/oct sub-bass crossover isolator (`40–300 Hz`), Sub Tightening clamp, and Sidechain Detector HPF (`20–500 Hz`) + spectral Tilt filter (`-6` to `+6 dB/oct`) with Sidechain Listen mode.
  - `TransientShaperModule` (`trans_*`): Differential dual-envelope transient designer for Kick Tok attack (`-100%` to `+100%`), sustain tail shaping (`-100%` to `+100%`), speed mode (`Fast` / `Punch`), and noise gate floor (`-90` to `-12 dB`).
  - `MultibandOTTModule` (`ott_*`): 3-band (`Low`, `Mid`, `High`) phase-coherent upward and downward compressor with `Depth`, `Time`, `Upward Drive`, `Downward Drive`, and `High Sheen` (`2.5 kHz+` Euphoric air boost).
  - `RhythmicPumperModule` (`pump_*`): Dual-mode rhythmic ducker supporting Host Tempo-Synced PPQ curves (`1/1` to `1/32`, Dotted/Triplet, plus *Duck*, *Hardstyle Reverse-Bass*, and *Pumping Swell* shapes) and Audio/External Sidechain envelope triggering, with full-spectrum or sub- ignored frequency scope.
  - `MaximizerClipperModule` (`clip_*`): Final loudness stage combining transient maximizer drive (`0` to `+18 dB`), adjustable ceiling (`-12.0` to `0.0 dBFS`), and 1st-order ADAA multi-curve clipping (`Hard Clip`, `Warm Tube`, `Hardstyle Crunch`).
- **Upgrades to Shared `WvZzModules` Components**:
  - `CompressorModule`: Adds selectable circuit topologies (`0: Opto Smooth`, `1: VCA Punch`, `2: FET Slam`) and soft/hard `Knee` (`0.0` to `12.0 dB`) while preserving 100% backward compatibility with `WvZz Distortion`.
  - `ModulationModule`: Adds configurable target profile support so `WvZz Distortion` keeps its 22 canonical targets while `WvZz Dynamics` exposes dynamics-specific modulatable targets (`Trans Attack`, `Trans Sustain`, `OTT Depth`, `OTT Upward`, `Comp Thresh`, `Pump Amount`, `Clip Drive`, `Master Mix`, `Master Vol`, etc.).
  - `SignalComparisonComponent` & `AudioVisualizerSource`: Adds real-time Gain Reduction trace and Crest Factor / Dynamic Range (dB) readout support (`GR / KNEE` mode) alongside `SCOPE`, `SPECTRUM`, and `DELTA`.
  - `HelpContentRepository`: Registers contextual `?` help documentation for all new dynamics modules and controls.

## Capabilities

### New Capabilities
- `dynamics-dsp-engine`: Defines the real-time zero-allocation audio processing pipeline for `WvZz Dynamics`, including the Linkwitz-Riley sub isolator and sidechain detector filter, differential transient shaper and gate, 3-band upward/downward OTT processor, multi-topology bus compressor, dual-mode rhythmic pumper, and ADAA maximizer clipper.

### Modified Capabilities
- `dsp-engine`: Extends the shared `Compressor` requirement to support selectable `Opto`, `VCA`, and `FET` circuit ballistics and adjustable soft `Knee` (`0–12 dB`).
- `modular-architecture`: Extends `ProcessContext` and `ModulationModule` to support plugin-specific modulation target profiles and gain-reduction telemetry without breaking existing distortion targets.
- `modulation-system`: Adds the Dynamics modulation target profile mapping LFO 1, LFO 2, and X/Y Pad axes to `WvZz Dynamics` parameters.
- `plugin-ui-system`: Defines the `WvZz Dynamics` 3-column dark obsidian card layout, luminous module accent colors, and signal-flow header bar.
- `signal-comparison-visualizer`: Adds real-time Gain Reduction history trace (`GR`) and live Crest Factor (dB) dynamic range telemetry to the visualizer.
- `component-help-system`: Extends the decoupled help repository with plain-language guidance for all `WvZz Dynamics` modules.
- `plugin-host-integration`: Registers the second plugin target (`WvZzDynamics`, code `WzDy`), stereo sidechain bus support, 20 factory presets, and automated `wvzz_dsp_tests` suites.

## Impact

- **Build & Packaging (`CMakeLists.txt`, `Makefile`)**: Adds `WvZzDynamics` target linked against `WvZzModules`, plus `wvzz_dsp_tests` coverage for all new modules and `WvZzDynamicsAudioProcessor`. `make package` automatically bundles both `WvZz Distortion` and `WvZz Dynamics` into the universal `.vst3` and `.app` release archives.
- **Source Code**:
  - `Source/Modules/DynDetector/`, `Source/Modules/TransientShaper/`, `Source/Modules/MultibandOTT/`, `Source/Modules/RhythmicPumper/`, `Source/Modules/MaximizerClipper/`
  - `Source/Modules/Compressor/`, `Source/Modules/Modulation/`, `Source/Core/UI/`, `Source/Core/DSP/`
  - `Plugins/WvZzDynamics/`
  - `Tests/DSPTests.cpp`
