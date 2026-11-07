# Design

## Context

See `proposal.md` for motivation and scope. `WvZzBundle` currently builds a shared static library (`WvZzModules`) and a single flagship plugin target (`WvZzDistortion`, code `WzDs`). All modules adhere to `IPluginModule` (`IDSPModule` + `IModuleView`), communicate exclusively via pre-allocated `ProcessContext` structures, and run under strict real-time audio invariants (zero heap allocations, zero mutexes, `noexcept` execution).

## Goals / Non-Goals

**Goals:**
- Introduce `WvZzDynamics` (`PLUGIN_CODE WzDy`) as a second macOS Universal Binary (`arm64;x86_64`) VST3 + Standalone target sharing `WvZzModules`.
- Implement 5 new headless-safe DSP & UI modules (`DynDetector`, `TransientShaper`, `MultibandOTT`, `RhythmicPumper`, `MaximizerClipper`) and upgrade `CompressorModule`, `ModulationModule`, and `SignalComparisonComponent`.
- Preserve 100% preset, parameter ID, and test compatibility for `WvZzDistortion`.

**Non-Goals:**
- Lookahead FFT linear-phase crossovers that introduce multi-millisecond latency during live MIDI/key-tracked performance. All crossovers use zero-latency phase-compensated Linkwitz-Riley (LR4 / 24 dB/oct) topologies derived from `CytomicSVF`.
- Audio Unit (AU) target builds (strictly VST3 and Standalone per repository policy).

## Decisions

### 1. Signal Flow & Pre-Allocated Buffer Topology in `WvZzDynamicsAudioProcessor`

```text
+----------------------------------------------------------------------------------+
|                        WvZzDynamicsAudioProcessor::processBlock                  |
+----------------------------------------------------------------------------------+
|                                                                                  |
|  Main Stereo In (Bus 0) -----> [MasterDSP::applyInputGain]                       |
|  Ext Sidechain In (Bus 1) ---+         |                                         |
|                              |         +--> [visualizerSource::pushInput]        |
|                              |         |                                         |
|                              v         v                                         |
|                        (1. DynDetectorModule: det_*)                             |
|                         - Filters Detector Buffer (HPF + Tilt)                   |
|                         - Splits Sub (< det_sub_xover) into auxBandRejectBuffer  |
|                         - Applies Sub Tightener to isolated Sub Band             |
|                                        |                                         |
|                                        v                                         |
|                        (2. TransientShaperModule: trans_*)                       |
|                         - Hysteresis Gate Floor + Fast/Slow Differential Shaper  |
|                                        |                                         |
|                                        v                                         |
|                        (3. MultibandOTTModule: ott_*)                            |
|                         - 3-Band LR4 Split (120 Hz / 2.5 kHz)                    |
|                         - Upward + Downward Compression + High Sheen             |
|                                        |                                         |
|                                        v                                         |
|                        (4. CompressorModule: comp_*)                             |
|                         - Opto / VCA / FET Glue using filtered Detector Buffer   |
|                                        |                                         |
|                                        v                                         |
|                        [Recombine Isolated Sub Band (if det_sub_lock == 1)]      |
|                                        |                                         |
|                                        v                                         |
|                        (5. RhythmicPumperModule: pump_*)                         |
|                         - PPQ Tempo-Synced Envelope OR Ext/Audio SC Trigger      |
|                         - Optional <180 Hz Sub-Only Ducking                      |
|                                        |                                         |
|                                        v                                         |
|                        (6. MaximizerClipperModule: clip_*)                       |
|                         - Maximizer Gain + 1st-Order ADAA (Hard / Tube / Crunch) |
|                                        |                                         |
|                                        v                                         |
|                        (7. MasterModule: master_*)                               |
|                         - Dry/Wet Mix, Output Gain, <120Hz Mono, 0 dBFS Guard    |
|                         - If det_sc_listen == 1: outputs Detector Buffer         |
|                                                                                  |
+----------------------------------------------------------------------------------+
```

- **Rationale:** Recombining the locked sub-bass band *after* the Transient Shaper, 3-Band OTT, and Bus Compressor—and *before* the Rhythmic Pumper and Maximizer Clipper—prevents sub-bass energy from causing mid/high distortion or OTT pumping while ensuring the sub still ducks rhythmically on the beat and hits the final brickwall ceiling cleanly.

### 2. Extending `ModTarget` and `ModulationModule` Without Breaking `WvZzDistortion`

- **Approach:** Keep indices `0..21` in `enum class ModTarget` identical for `WvZzDistortion`. Append Dynamics targets (`DynSubXover = 22`, `DynScHp`, `DynTransAttack`, `DynTransSustain`, `DynOttDepth`, `DynOttUpDrive`, `DynOttSheen`, `DynCompThresh`, `DynCompMix`, `DynPumpAmount`, `DynPumpCurve`, `DynClipDrive`) and increase `ModulationDSP::numTargets` to `34`.
- `ModulationModule` and `ModulationDSP` accept a `ModProfile` enum (`ModProfile::Distortion` default, or `ModProfile::Dynamics`). When `ModProfile::Dynamics` is selected, the UI dropdowns (`lfo1_target`, `lfo2_target`, `xy_x_target`, `xy_y_target`) register the 16-item Dynamics target list and map choice indices to the corresponding `ModTarget` enum slot in `targetOffsetsCurrent`.
- **Rationale:** Zero breaking changes to `WvZzDistortion` state recall or unit tests, while `ProcessContext::getModulationOffset(ModTarget)` remains a single $O(1)$ array lookup.

### 3. Multi-Topology Upgrade to `CompressorDSP`

- **Approach:** Add `comp_mode` (`0: Opto`, `1: VCA`, `2: FET`, default `0`) and `comp_knee` (`0.0` to `12.0 dB`, default `0.0 dB` in direct `setParameters` default so existing unit tests stay bit-identical, `3.0 dB` APVTS default).
  - **Opto (`0`):** Preserves the exact dual-stage optical release ballistics (`fastEnvelopeDb` + `slowEnvelopeDb`) already verified in `DSPTests.cpp`.
  - **VCA (`1`):** Uses single-stage RMS-style decibel ballistics with soft-knee quadratic interpolation around `comp_thresh ± comp_knee / 2` for transparent Big Room bus glue.
  - **FET (`2`):** Scales attack time by `0.2x` (down to `20 µs`), uses peak detection, and adds subtle odd-harmonic grit when Phat mode is enabled.

### 4. Mathematical Models for New Dynamics Modules

1. **`DynDetectorDSP` (`det`):**
   - Sub Crossover: Cascaded pair of 2-pole `CytomicSVF` filters per channel forming a 24 dB/oct Linkwitz-Riley low-pass/high-pass split (`Low = LP2(LP1(x))`, `High = x - Low` phase-compensated).
   - Sub Tightener: Tracks sub RMS envelope and applies dynamic downward clamping to sustain tails when `det_sub_tight > 0.0`.
   - Sidechain Filter: 2-pole HPF at `det_sc_hp` (`20–500 Hz`) + 1-pole Tilt shelf around `1 kHz` (`det_sc_tilt` `-6` to `+6 dB`).
2. **`TransientShaperDSP` (`trans`):**
   - Dual peak envelope followers in linear/dB domain: `envFast` (`0.3 ms` attack, `15 ms` release) and `envSlow` (`15 ms` attack, `150 ms` release; scaled `1.8x` in `Punch` mode).
   - Normalized differential `diff = (envFast - envSlow) / (envSlow + 1e-4f)`. Positive `diff` scales by `trans_attack` (`±12 dB`); negative `diff` (decay tail) scales by `trans_sustain` (`-18` to `+12 dB`).
   - Hysteresis Noise Gate: Smoothly attenuates audio below `trans_gate` (`-90` to `-12 dB`) with a `3 ms` open and `40 ms` close ballistic ramp.
3. **`MultibandOTTDSP` (`ott`):**
   - Splits stereo signal into 3 bands (`Low < 120 Hz`, `Mid 120 Hz – 2.5 kHz`, `High > 2.5 kHz`) using zero-latency `CytomicSVF` crossover pairs.
   - Each band runs a fast RMS envelope detector (`envDb`).
   - **Downward region (`envDb > -12.0 dB`):** Attenuates peaks above threshold scaled by `ott_down_drive`.
   - **Upward region (`-55.0 dB < envDb < -22.0 dB`):** Boosts quiet signals up to `+18 dB * ott_up_drive`, tapering smoothly to `0 dB` below `-55.0 dB` to protect the noise floor.
   - **High Sheen:** Applies a pre-compression `2.5 kHz` high-shelf boost (`0` to `+12 dB`) on the High band so upward compression pulls out shimmering Euphoric air.
4. **`RhythmicPumperDSP` (`pump`):**
   - **Tempo Sync Mode (`0`):** Computes normalized beat phase $\phi \in [0, 1)$ from `ctx.hostPpqPosition` (or internal phase accumulator at `ctx.hostBpm`).
   - **Audio / Ext SC Mode (`1`):** Detects rising transients on `ctx.sidechainInputBuffer` exceeding `-24 dBFS` and sweeps $\phi$ from `0.0` to `1.0` at the selected rate duration.
   - **4 Contour Shapes:**
     - `0: Classic Duck`: Smooth cosine-squared sidechain dip at beat start, recovering by $\phi = \text{pump\_curve}$.
     - `1: Hard Kick Clear`: Instantaneous `-inf` dip for the first `15%` of the cycle (kick Tok clearance) followed by S-curve recovery.
     - `2: Reverse Bass`: Off-beat Hardstyle reverse-bass envelope that ducks the beat impact and swells with a resonant parabolic peak at $\phi \approx 0.65$ before cutting off cleanly before the next kick.
     - `3: Festival Swell`: Exponential build-up curve ideal for Euphoric pads and supersaw risers.
5. **`MaximizerClipperDSP` (`clip`):**
   - Applies input gain `clip_drive` (`0` to `+18 dB`) into ceiling `clip_ceiling` (`-12.0` to `0.0 dBFS`).
   - Normalized sample $u = x / \text{ceiling}$ passes through 1st-order ADAA waveshaper:
     - `0: Hard Clip`: ADAA of $\text{clamp}(u, -1, 1)$ with quadratic antiderivative $F(u) = \frac{1}{2}u^2$ for $|u| \le 1$ and $|u| - \frac{1}{2}$ for $|u| > 1$.
     - `1: Warm Tube`: ADAA of $\tanh(u)$ using $F(u) = \ln(\cosh(u))$.
     - `2: Hardstyle Crunch`: Asymmetric ADAA combining hard positive transient saturation with folded harmonic inflection in the knee region (`0.7 < |u| < 1.2`) before clamping at $\pm\text{ceiling}$.

## Risks / Trade-offs

- **[Risk] Multiband phase cancellation when mixing dry/wet in OTT or Sub Crossover** → **Mitigation:** Use matched-topology complementary subtraction (`High = Input - Low`) on `CytomicSVF` state variable filters so that summing bands at unity gain reconstructs the input with flat magnitude and phase response.
- **[Risk] Upward OTT amplifying silence between notes** → **Mitigation:** Place `TransientShaperModule` (with its hysteresis Noise Gate floor) immediately before `MultibandOTTModule`, and taper upward OTT gain back to `0 dB` below `-55 dBFS`.
- **[Risk] UI crowding with 6 dynamics modules + Modulation + Master** → **Mitigation:** Use the proven `1080 x 680` 3-column layout with compact 4-to-6 knob module cards, pill buttons for modes, and luminous accent colors per module (`DynDetector`: Warm Amber, `TransientShaper`: Emerald Green, `MultibandOTT`: Sky Cyan, `Compressor`: Honey Gold, `RhythmicPumper`: Electric Violet, `MaximizerClipper`: Coral Red, `Master`: Arctic Ice White).
