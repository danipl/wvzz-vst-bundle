# Design

## Context

See `proposal.md` for motivation and scope. The existing codebase in `Source/` provides a lock-free JUCE 8 `AudioProcessor` and custom `LookAndFeel` editor, but uses prototype parameter names, simplified DSP equations, post-Flanger band-reject summing, only 3 MM Filter modes, a 5-knob compressor, no LFO modulation engine, no oversampling dry latency compensation, and a fixed `920x600` layout.

All real-time constraints from `plugin-host-integration` remain mandatory: strictly zero heap allocations in `processBlock()`, `juce::ScopedNoDenormals`, and sample-rates from `44.1 kHz` to `192 kHz`.

## Goals / Non-Goals

**Goals:**
- Implement exact DSP equations and parameter tables for all 7 modules and the Modulation System (LFO 1, LFO 2, X/Y Pad).
- Guarantee phase-coherent parallel dry/wet mixing (`master_mix`) and band-reject recombination (`bp_br_mix`).
- Maintain zero heap allocations on the audio thread across all 12 MM Filter topologies and 32-sample control-rate modulation updates.
- Support proportional `900 x 560 px` (`75%`–`200%`) UI scaling using a clean root content component with `juce::AffineTransform::scale`.

**Non-Goals:**
- Changing the existing `component-help-system` architecture (we only update `HelpContentRepository.h` text to reflect the new controls and LFO/XY panels).
- Supporting non-macOS platforms or external preset file browsers beyond host program management and XML/JSON state serialization.

## Decisions

### 1. Control-Rate Modulation Engine & Parameter Resolution
- **Decision**: Create `Source/DSP/ModulationEngine.h` that evaluates LFO 1, LFO 2, and X/Y Pad offsets every 32 samples (or the remaining sub-block length when `numSamples % 32 != 0`) and computes per-sample linear ramps for all 21 `ModTarget` entries.
- **Rationale**: Computing transcendental waveform functions (`std::sin`, cubic S&H) and frequency octave scaling (`std::pow(2.0f, octaves)`) once every 32 samples reduces CPU overhead by ~95% while linear interpolation over 32 samples (`< 0.7 ms` at 44.1 kHz) eliminates stepping artifacts.
- **Cross-Modulation (`LFO 1 -> LFO 2 Rate`)**: LFO 1 is evaluated first in each 32-sample tick; if `lfo1_target == ModTarget::LFO2Rate`, LFO 1's bipolar output (`[-1, 1] * lfo1_depth`) modulates LFO 2's effective rate (`rate * 2^(offset * 3.0)`) before LFO 2 advances its phase.
- **Alternatives considered**: Per-sample LFO evaluation (unnecessary CPU cost for 21 targets) or once-per-buffer evaluation (causes audible stepping at buffer sizes ≥ 256 samples).

### 2. Signal Flow & Pre-BP Envelope / Dry Latency Buffers
- **Decision**: In `CamelPhatAudioProcessor::processBlock()`, after applying `input_gain`:
  1. Copy the input-gained buffer into `preBpInputBuffer` (used by `MMFilter`'s Envelope Follower so distortion drive never alters filter envelope dynamics) and `globalDryBuffer`.
  2. Run `BPFilter::process()` producing `passbandBuffer` and `bandRejectBuffer` (`Low_Rejected + High_Rejected`).
  3. Run `DistortionModule::process(passbandBuffer)` (base-rate `Bitcrusher` followed by 4x oversampled parallel `Mech`, `Tube`, and `Xcita`).
  4. Run `MMFilter::process(passbandBuffer, preBpInputBuffer)`.
  5. Sum `bandRejectBuffer * bp_br_mix` into `passbandBuffer` **before** `FlangerModule`.
  6. Run `FlangerModule`, `MagicEQ`, and `CompressorModule` in series on `passbandBuffer`.
  7. Pass `globalDryBuffer` through a pre-allocated fractional/integer delay line in `MasterSection` matching `distortionModule.getLatencyInSamples()` (when `dist_bypass == 0`), then blend via `master_mix`, apply `master_vol`, and crossfade with `master_bypass`.
- **Rationale**: Matches the Section 2 topology diagram and Section 5 latency compensation requirement verbatim.

### 3. Module 1 (BP Filter) Cytomic ZDF SVF & `bp_link`
- **Decision**: Implement Andy Simper's trapezoidal integrated ZDF SVF directly in `BPFilter.h` (`g = tan(pi * fc / fs)`, `k = 1 / Q`, solving `v1`, `v2`, `v3` per sample to obtain simultaneous `lp = v2`, `bp = v1`, `hp = input - k*v1 - v2`).
  - Filter 1 (`Low Cut` at `bp_low_cut`, `Q` from `bp_low_res` in `[0.707, 12.0]`): outputs `Low_Rejected = lp1` and `hp1`.
  - Filter 2 (`High Cut` at `bp_high_cut`, `Q` from `bp_high_res` in `[0.707, 12.0]`): processes `hp1` to output `Passband = lp2`, and processes `Input` (or `hp1`) via a dedicated high-cut SVF state to extract `High_Rejected = hp2(Input)`.
  - `bp_link`: Track the last normalized/log difference between `bp_low_cut` and `bp_high_cut` in `CamelPhatAudioProcessor`; when `bp_link == 1` and one cutoff changes, update the linked cutoff preserving $\log_2(f_{\text{high}} / f_{\text{low}})$.

### 4. Module 3 (MM Filter) 12 Topologies in Pre-Allocated Memory
- **Decision**: Implement all 12 modes inside `MMFilter.h` without virtual dispatch or dynamic allocation in `process()`:
  - **Modes 0–7 (`LP 12`, `LP 24`, `BP 12`, `BP 24`, `HP 12`, `HP 24`, `Notch`, `Peak`)**: Dual cascaded Cytomic ZDF SVF stages per channel. For `LP 24dB`, apply `std::tanh` saturation to the integrator feedback state (`ic1eq`, `ic2eq`) to model ladder self-oscillation limiting.
  - **Modes 8–9 (`Comb +`, `Comb -`)**: Pre-allocate a stereo circular buffer of `ceil(maxSampleRate / 20.0) + 8` floats in `prepare()`. Delay time in samples is `fs / cutoffHz`, read via 3rd-order Lagrange interpolation, with feedback `±(0.1f + 0.88f * mm_res)` soft-clipped by `std::tanh`.
  - **Mode 10 (`RingMod`)**: Multiply input by `std::sin(ringPhase)` advancing at `2 * pi * cutoffHz / fs`, with resonant feedback `mm_res * 0.85f * std::tanh(prevOut)` added to the carrier input.
  - **Mode 11 (`Vowel / Formant`)**: 3 parallel 2-pole ZDF bandpass filters per channel tuned to formant frequencies $(F_1, F_2, F_3)$ linearly interpolated across the 5 classic vowels (`A`, `E`, `I`, `O`, `U`) using the normalized log position of `mm_cutoff` (`20 Hz` → `A`, `20 kHz` → `U`), with filter `Q` scaled by `mm_res` (`4.0` to `18.0`).

### 5. Resizable `900 x 560` GUI Architecture
- **Decision**: Use a fixed logical coordinate canvas (`900 x 560 px`) inside a container `Component` (`contentContainer`) owned by `CamelPhatAudioProcessorEditor`. Set `setResizable(true, true)` and `setResizeLimits(675, 420, 1800, 1120)` with `getConstrainer()->setFixedAspectRatio(900.0 / 560.0)`. In `resized()`, apply `juce::AffineTransform::scale(getWidth() / 900.0f, getHeight() / 560.0f)` to `contentContainer`.
- **Rationale**: Guarantees pixel-perfect vector scaling at any zoom factor from `75%` to `200%` without integer rounding drift across 40+ child controls.

## Risks / Trade-offs

- **[Risk] High resonance + Phat saturation causing DC offset or supersonic ringing** → **Mitigation**: Keep `10 Hz` DC blockers active in Distortion, clamp ZDF SVF cutoff frequencies to `0.49 * sampleRate`, and wrap feedback paths in `std::tanh`.
- **[Risk] Host automation loop when `bp_link` updates the companion cutoff parameter** → **Mitigation**: Use a `std::atomic<bool> isUpdatingLinkedCutoff` re-entrancy guard when synchronizing `bp_low_cut` and `bp_high_cut`.
