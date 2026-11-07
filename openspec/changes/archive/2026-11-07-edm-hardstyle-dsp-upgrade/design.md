# Design

## Context

`WvZzDistortion` processes stereo audio in 32-sample control-rate sub-blocks through 8 headless `IDSPModule` instances orchestrated via `ProcessContext`. See `proposal.md` for motivation and `specs/` for behavioral requirements. All changes must preserve the real-time audio invariants: zero heap allocations, zero locks, `noexcept` execution in `processBlock()`, and multi-instance parameter prefixing (`instancePrefix`).

## Goals / Non-Goals

**Goals:**
- Propagate live host transport (`bpm`, `ppqPosition`, `isPlaying`) and active distortion oversampling latency (`distortionLatencySamples`) through `ProcessContext` without heap allocation.
- Eliminate phase cancellation across both the Band-Reject recombination node (`BPFilterDSP`) and the Master dry/wet mixer (`MasterDSP`).
- Upgrade `DistortionDSP` from additive parallel dry summation to serial cascaded saturation with selectable `Classic`, `Hard Clip`, and `Wavefold` character modes.
- Allow `MagicEQDSP` to operate either Pre-Distortion (`meq_pre = 1`) for Hardstyle kick-tok resonant drive or Post-Flanger (`meq_pre = 0`) with an extended `45–180 Hz` tune range.
- Add `< 120 Hz` Sub-Bass Mono summing (`master_mono_bass`), `0.0 dBFS` Output Clipper (`master_clip`), click-free `master_bypass` crossfading, and `ModTarget::MasterVol` modulation to `MasterDSP`.
- Provide 5 new factory presets tailored for Hardstyle, Euphoric Hardstyle, and Big Room EDM.

**Non-Goals:**
- Changing the fixed `1080 x 680` editor aspect ratio or 3-column card layout.
- Introducing FFT-based or high-latency linear-phase crossovers that would increase plugin latency beyond the 4x oversampler's group delay (~3–12 samples).

## Decisions

### 1. Transport and Latency Metadata in `ProcessContext`
- **Choice**: Add `double hostBpm { 120.0 }`, `double hostPpqPosition { 0.0 }`, `bool isHostPlaying { false }`, and `int distortionLatencySamples { 0 }` with default member initializers to `ProcessContext`.
- **Rationale**: Default member initializers keep existing unit-test aggregate initializations of `ProcessContext` backward-compatible while letting `WvZzDistortionAudioProcessor::processBlock` pass sub-block-accurate transport and latency values to every module without locks or allocations.
- **Beat-Phase Lock**: When `lfoX_sync` is enabled and `isHostPlaying` is true, `ModulationDSP` computes LFO phase directly from `std::fmod(hostPpqPosition / beatsPerCycle, 1.0) * twoPi` (with sample-offset advancement across sub-blocks) so LFO sweeps always align with the 4/4 kick downbeat. When transport is stopped, it advances phase freely at the synced `hostBpm` rate.

### 2. Serial Cascaded Distortion with Selectable `Classic / Hard Clip / Wavefold` Modes
- **Choice**: Inside `DistortionDSP::process` (4x oversampled block):
  1. `Mech` processes the signal and blends `x = (1.0f - mechMix) * x + mechMix * mechOut` (where `mechMix = std::sqrt(modMech)` for strong authority across the knob range).
  2. `Tube` processes the cascaded `x` and blends `x = (1.0f - tubeMix) * x + tubeMix * tubeOut`.
  3. Character shaper (`dist_mode`) shapes the combined `Mech + Tube` signal when `hasMech || hasTube`:
     - `0 (Classic)`: Warm asymmetric/tanh transfer with soft ceiling.
     - `1 (Hard Clip)`: Bipolar hard clipper (`juce::jlimit(-0.95f, 0.95f, x * (1.0f + 1.5f * maxDrive))`) that shaves transient peaks flat for Hardstyle kick "tok" and Big Room drums.
     - `2 (Wavefold)`: Sine wavefolder (`std::sin(x * foldGain)`) blended according to drive, folding peaks inward to generate aggressive metallic harmonics for screech leads.
  4. `Xcita` extracts high frequencies (`> 2.8 kHz`), rectifies/saturates them, and adds harmonic air on top before a final oversampled soft safety limiter (`std::tanh(x * 0.5f) * 2.0f`).
- **Alternatives Considered**: Keeping pure parallel addition (`out = dry + mech + tube`). Rejected because clean transients always poke through unclipped, preventing flat-topped Hardstyle kick tails.

### 3. Band-Reject Phase Coherence and Oversampling Delay Compensation
- **Choice**:
  - In `BPFilterDSP::process`: Compute `lowRejected = svfL.lp`, `highPassed = x - lowRejected`, `passband = svfH.lp(highPassed)`, and `highRejected = highPassed - passband`. Because `lowRejected + passband + highRejected == x` identically when recombining at unity without distortion, crossover phase cancellation is eliminated.
  - Pre-allocate a stereo circular delay buffer (`rejDelayBufferL/R`, 256 samples) in `BPFilterDSP::prepare`. In `BPFilterDSP::recombine(ProcessContext& ctx)`, delay `auxBandRejectBuffer` by `ctx.distortionLatencySamples` before summing into `ctx.mainBuffer`.
  - In `DistortionDSP::process`: When `bypassed` or all drives are `0.0`, set `ctx.distortionLatencySamples = 0` (and when oversampling runs, set `ctx.distortionLatencySamples = getLatencyInSamples()`). `MasterDSP` also reads `ctx.distortionLatencySamples` for its dry delay tap so `master_mix` is always sample-aligned whether distortion is active or bypassed.

### 4. Pre/Post Distortion Magic EQ Routing (`meq_pre`)
- **Choice**: Add `meq_pre` (`AudioParameterBool`, default `false`) and widen `meq_tune` to `45.0–180.0 Hz` (default `80.0 Hz`). In `WvZzDistortionAudioProcessor::processBlock`, check `meqModule.getMagicEQDSP().isPreDistortion()`:
  - If `true`, invoke `meqModule.getDSP().process(ctx)` right after `bpModule` and before `distModule`.
  - If `false`, invoke `meqModule.getDSP().process(ctx)` in its standard position after `flgModule`.
- **Rationale**: Zero extra CPU cost (still runs once per sub-block), preserves single-instance APVTS parameter IDs (`meq_*`), and enables authentic Hardstyle kick design where a resonant `50–160 Hz` EQ peak drives directly into the clipper/waveshaper.

### 5. Master Sub-Bass Mono Maker, 0 dBFS Clipper, and Click-Free Bypass
- **Choice**:
  - **Click-Free Bypass**: Smooth `master_bypass` with a `juce::SmoothedValue<float>` (`bypassSmoothed`, 10 ms ramp) and crossfade between `wet` and `delayedDry` per sample.
  - **Sub-Bass Mono Maker (`master_mono_bass`)**: Use two matched 2-pole Cytomic SVF filters (`120.0 Hz`, $Q = 0.7071$) on Left and Right channels. Extract `lowL = svfL.lp`, `lowR = svfR.lp`, compute `monoLow = 0.5f * (lowL + lowR)`, and reconstruct `outL = (inL - lowL) + monoLow`, `outR = (inR - lowR) + monoLow`. Subtracting `lowL/R` from `inL/R` guarantees flat magnitude and phase reconstruction when `L == R` while collapsing `< 120 Hz` stereo side energy to mono.
  - **0 dBFS Clipper (`master_clip`, default `true`)**: When enabled, apply a soft-knee saturating clipper (`x / (1.0f + std::abs(x))` or hard-knee `std::tanh` / clamp at `±1.0f` / `0.0 dBFS`) after output volume and `ModTarget::MasterVol` modulation.

## Risks / Trade-offs

- **[Risk] Existing tests check `numPrograms == 10` or specific `ModTarget` count** → **Mitigation**: Update `DSPTests.cpp` to assert 15 factory presets and 22 `ModTarget` entries, plus add dedicated unit tests for all new DSP paths.
- **[Risk] Changing `meq_tune` range from `60–120 Hz` to `45–180 Hz` could shift normalized values if presets used 0..1 values** → **Mitigation**: `FactoryPresets.h` stores raw Hz values (`75.0f`, `85.0f`, `65.0f`, `80.0f`) and converts via `param->convertTo0to1(val)`, so all existing presets recall at their exact Hz frequencies.
