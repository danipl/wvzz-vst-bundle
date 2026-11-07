# Proposal

## Why

`WvZzDistortion` currently suffers from five DSP defects (hardcoded 120 BPM LFO tempo sync, purely additive parallel saturation that never clips signal peaks, uncompensated oversampling group delay on the Band-Reject sub-bass path, dry/wet comb filtering when distortion knobs are at zero, and unsmoothed master bypass / `+18 dBFS` output spikes) and lacks key waveshaping, routing, and stereo-control features needed for EDM, Big Room, and Euphoric/Raw Hardstyle production. Fixing these DSP defects and adding serial hard-clip/wavefold saturation, pre-distortion resonant EQ routing, beat-locked LFO pumping, and sub-bass mono summing turns the plugin into a precision sound-design tool for Hardstyle kicks, screeches, reverse basses, and festival leads.

## What Changes

- **Fix Host Tempo Sync & Beat-Phase Lock (`ModulationDSP`, `ProcessContext`, `PluginProcessor`)**:
  - Pass live DAW transport `bpm`, `ppqPosition`, and `isPlaying` state through `ProcessContext` on every sub-block so tempo-synced LFOs run at the actual host tempo (e.g., 128 BPM Big Room, 150 BPM Hardstyle) and lock phase to the beat grid during playback.
  - Add `MasterVol` (`Master Volume / Pump`) as a new modulation target (`ModTarget::MasterVol`, expanding the target list to 22 items) for beat-synced reverse-bass swells, rhythmic pumping, and trance gating.
- **Upgrade 4-Stage Distortion to Serial Cascading with Clip & Wavefold Modes (`DistortionDSP`, `DistortionView`)**:
  - Replace the weak parallel-only dry addition in `DistortionDSP` with serial (cascaded) wet/dry stage blending so `Mech`, `Tube`, and `Xcita` actively compress and saturate waveform peaks.
  - Add a `dist_mode` parameter (`0: Classic`, `1: Hard Clip`, `2: Wavefold`) inside the 4x oversampled loop to produce flat peak-shaved transients for Big Room/Hardstyle kicks and metallic folded harmonics for Hardstyle screech leads.
  - Keep 4x oversampling latency consistent (or dynamically matched in `ProcessContext`) when distortion drives are zero so `master_mix` never suffers from comb-filtering phase cancellation.
- **Fix Band-Pass / Band-Reject Phase & Latency Alignment (`BPFilterDSP`)**:
  - Compute the rejected band as a phase-aligned complement (`lowRejected + highRejected`) and delay `auxBandRejectBuffer` by the active 4x oversampling group delay before recombining in `BPFilterDSP::recombine()`, preserving sub-bass punch at the crossover frequency.
- **Add Pre-Distortion Routing & Extended Kick-Tok Range to Magic EQ (`MagicEQDSP`, `MagicEQView`, `PluginProcessor`)**:
  - Add a `meq_pre` boolean parameter (`PRE` pill toggle in `MagicEQView`) that switches `MagicEQ` between **Pre-Distortion** (immediately after `BPFilter` to push a resonant `45–180 Hz` punch peak directly into `Distortion` for Hardstyle kick tails) and **Post-Flanger** (classic sub-bass weight enhancer).
  - Expand `meq_tune` range from `60–120 Hz` to `45–180 Hz` to cover both deep sub fundamentals (`45–55 Hz` for G/A Hardstyle kicks) and upper kick "tok" resonances (`120–180 Hz`).
- **Add Sub-Bass Mono Maker, 0 dBFS Clipper, and Click-Free Bypass to Master (`MasterDSP`, `MasterView`)**:
  - Add a click-free smoothed crossfade ramp (`masterBypassSmoothed`) when toggling `master_bypass`.
  - Add a `master_clip` toggle (`CLIP` pill in `MasterView`, default on) applying a `0.0 dBFS` ceiling clipper (`tanh` soft/hard ceiling) after output gain to prevent `+18 dBFS` resonant spikes from overloading the DAW bus.
  - Add a `master_mono_bass` toggle (`MONO <120Hz` pill in `MasterView`) using a 120 Hz Linkwitz-Riley / 2-pole crossover that sums stereo low-end below `120 Hz` to mono while keeping mids and highs wide.
- **Add 5 EDM, Big Room & Hardstyle Factory Presets (`FactoryPresets.h`)**:
  - Expand factory presets from 10 to 15 with `Hardstyle Kick Tok & Crunch`, `Euphoric Supersaw Excite`, `Raw Screech Comb Screamer`, `Reverse Bass Pump`, and `Big Room Festival Kick`.

## Capabilities

### New Capabilities
<!-- None: all changes extend existing cohesive capabilities -->

### Modified Capabilities
- `dsp-engine`: Fix Band-Reject oversampling latency and phase alignment; upgrade 4-stage distortion to serial cascading with selectable `Classic / Hard Clip / Wavefold` modes and constant latency alignment; extend `Magic EQ` tune to `45–180 Hz` and add Pre/Post Distortion routing (`meq_pre`); add click-free `master_bypass`, `0 dBFS` Master Clipper (`master_clip`), and `< 120 Hz` Sub-Bass Mono Maker (`master_mono_bass`).
- `modulation-system`: Propagate live DAW `bpm` and `ppqPosition` to lock synced LFOs to the host tempo and beat grid; expand the Modulatable Targets Enum to 22 items by adding `MasterVol` (`Master Vol / Pump`).
- `modular-architecture`: Extend `ProcessContext` with zero-allocation transport metadata (`bpm`, `ppqPosition`, `isPlaying`) and active distortion latency samples (`distortionLatencySamples`).
- `plugin-host-integration`: Expand built-in factory presets from 10 to 15 to include EDM, Big Room, and Euphoric/Raw Hardstyle presets and add automated verification tests for the new DSP features.
- `plugin-ui-system`: Add UI controls for Distortion Mode (`dist_mode`), Magic EQ Pre-Distortion routing (`meq_pre`), Master Output Clipper (`master_clip`), and Master Sub-Bass Mono (`master_mono_bass`), and update the header signal-flow indicator when `meq_pre` is active.

## Impact

- **Core Interfaces**: `Source/Core/Interfaces/ProcessContext.h` (`ModTarget` enum and `ProcessContext` struct fields).
- **DSP & UI Modules**:
  - `Source/Modules/BPFilter/BPFilterDSP.h/.cpp` (delay line for `auxBandRejectBuffer` oversampling latency alignment).
  - `Source/Modules/Distortion/DistortionDSP.h/.cpp` & `DistortionView.h/.cpp` (`dist_mode`, serial cascading, constant oversampling delay).
  - `Source/Modules/MagicEQ/MagicEQDSP.h/.cpp` & `MagicEQView.h/.cpp` (`meq_pre` toggle, `45–180 Hz` tune range).
  - `Source/Modules/Modulation/ModulationDSP.h/.cpp` (host `bpm` & `ppqPosition` beat sync, `ModTarget::MasterVol`).
  - `Source/Modules/Master/MasterDSP.h/.cpp` & `MasterView.h/.cpp` (click-free bypass ramp, `master_clip`, `master_mono_bass`, `ModTarget::MasterVol` modulation).
- **Plugin Host & Presets**:
  - `Plugins/WvZzDistortion/PluginProcessor.cpp` (pass transport BPM/PPQ, Pre/Post Magic EQ routing).
  - `Plugins/WvZzDistortion/Presets/FactoryPresets.h` (15 factory presets).
- **Verification**: `Tests/DSPTests.cpp` (automated tests for BPM sync, serial clip/wavefold distortion, BR latency alignment, Pre-Distortion Magic EQ, Bass Mono maker, Master Clipper, and 15 presets).
