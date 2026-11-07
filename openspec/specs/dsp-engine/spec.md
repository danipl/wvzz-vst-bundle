# dsp-engine Specification

## Purpose
Defines the real-time audio digital signal processing pipeline recreating the CamelPhat 3 multi-effect processor signal topology and sonic characteristics.

## Requirements

### Requirement: Band-Pass Splitter Routing
The DSP engine SHALL split incoming stereo audio into a Passband signal and a phase-aligned Rejected Band signal using two 12 dB/oct (2-pole) Cytomic zero-delay feedback (ZDF) trapezoidal State Variable Filters (one High-Pass at `bp_low_cut` with `bp_low_res`, one Low-Pass at `bp_high_cut` with `bp_high_res`, Q mapped `0.707` to `12.0`), where `Low_Rejected = LowPass_Output(Input, bp_low_cut, bp_low_res)`, `HighPassed = Input - Low_Rejected` (or `HighPass_Output`), `Passband = LowPass_Output(HighPassed, bp_high_cut, bp_high_res)`, and `High_Rejected = HighPassed - Passband` so that `Low_Rejected + Passband + High_Rejected` preserves phase coherence across crossover points, and `bp_link` locks the logarithmic frequency delta between `bp_low_cut` and `bp_high_cut`. The engine and parameter controls SHALL enforce that `bp_low_cut` never exceeds `bp_high_cut` ($f_{\text{low}} \le f_{\text{high}}$), and companion parameter synchronization under `bp_link` SHALL NOT invoke host parameter notifications from the real-time audio thread.

#### Scenario: Audio band splitting
- **WHEN** audio is processed through the BP Filter splitter with `bp_bypass = 0`
- **THEN** frequencies within the passband cascade pass to the Distortion stage while the phase-coherent `Low_Rejected + High_Rejected` signal is extracted into the auxiliary band-reject buffer for latency-compensated post-MM-Filter recombination

#### Scenario: Linked cutoff movement
- **WHEN** `bp_link` is enabled (`1`) and either `bp_low_cut` or `bp_high_cut` is adjusted
- **THEN** the companion cutoff updates automatically to preserve the logarithmic frequency ratio within `[20.0 Hz, 20000.0 Hz]` without audio-thread host notification calls

#### Scenario: Cutoff boundary clamping prevents crossover
- **WHEN** user or automation adjusts `bp_low_cut` upward toward `bp_high_cut` or `bp_high_cut` downward toward `bp_low_cut`
- **THEN** the values are clamped such that `bp_low_cut` does not exceed `bp_high_cut`, preventing passband silencing and out-of-band double summation

### Requirement: 4-Stage Oversampled Distortion
The system SHALL process the passband audio through 4 cascaded distortion circuits (`Bitcrusher`, `Mech`, `Tube`, and `Xcita`) where `Bitcrusher` applies amplitude quantization (`16-bit` to `4-bit`) and sample-and-hold decimation (`1x` to `32x`) at the base sample rate before upsampling, and `Mech` (`1.2 kHz` +6 dB Q=1.0 pre-emphasis, asymmetric transfer `(x >= 0) ? tanh(drive * x) + 0.15 * sin(drive * x * pi) : clamp(drive * 1.2 * x, -0.9, 0.0)`, post-de-emphasis, exp drive `1.0`–`25.0`), `Tube` (`tanh(drive * x + 0.1 * x^2) - tanh(0.1 * x^2)` with DC blocker, exp drive `1.0`–`15.0`), and `Xcita` (`2.8 kHz` 2-pole Butterworth HPF, `tanh(drive * (0.7 * x_hf + 0.3 * abs(x_hf)))`, `10 Hz` DC blocker) run in series inside a 4x polyphase IIR/FIR oversampling block with selectable character shaping (`dist_mode`: `0: Classic`, `1: Hard Clip`, `2: Wavefold`) and consistent group delay alignment.

#### Scenario: Parallel distortion saturation
- **WHEN** distortion amounts for `dist_mech`, `dist_tube`, `dist_bit`, or `dist_xcita` are increased above zero with `dist_bypass = 0`
- **THEN** each active stage crossfades and cascades into the next stage inside the 4x oversampled loop, generating dense harmonic saturation without dry-transient bleed or aliasing

#### Scenario: Selectable Hard Clip and Wavefold distortion character modes
- **WHEN** `dist_mode` is set to `1 (Hard Clip)` or `2 (Wavefold)` with active `dist_mech` or `dist_tube` drive
- **THEN** `Hard Clip` clamps driven peaks to a flat bipolar ceiling for aggressive kick transients, while `Wavefold` folds driven peaks via a trigonometric folding transfer curve to generate metallic upper harmonics for screech leads

#### Scenario: Zero-drive latency consistency
- **WHEN** `dist_bypass` is `1` or all distortion drive parameters are `0.0`
- **THEN** the reported and compensated oversampling latency in `ProcessContext` matches the actual wet-path group delay so that `master_mix` and `bp_br_mix` produce zero comb-filtering phase cancellation

### Requirement: Modulated Multi-Mode Filter
The system SHALL provide a multi-mode filter (`MM Filter`) after the Distortion stage supporting all 12 topologies (`0: LP 12dB`, `1: LP 24dB` with internal `tanh` feedback saturation, `2: BP 12dB`, `3: BP 24dB`, `4: HP 12dB`, `5: HP 24dB`, `6: Notch`, `7: Peak`, `8: Comb +`, `9: Comb -`, `10: RingMod`, `11: Vowel / Formant` morphing across `A-E-I-O-U` formant tables via `mm_cutoff`), with wet/dry blend (`mm_mix`) and cutoff modulation (`mm_env_depth` in `±6 octaves`) driven by an Envelope Follower (`mm_env_att` `0.1`–`1000 ms`, `mm_env_rel` `1.0`–`5000 ms`) tapped from the **Plugin Input (pre-BP Filter)**.

#### Scenario: Envelope follower cutoff modulation
- **WHEN** audio enters the plugin input and `mm_env_depth` is non-zero
- **THEN** the envelope follower tracks the pre-BP input level using decoupled attack/release ballistics and shifts `mm_cutoff` across a `±6 octave` range regardless of distortion gain changes

#### Scenario: 12-topology filter selection
- **WHEN** `mm_type` is switched to any index from `0` to `11`
- **THEN** the MM Filter processes audio using the selected SVF, Ladder, Comb, RingMod, or Formant vowel topology blended by `mm_mix`

### Requirement: Stereo Modulated Flanger
The system SHALL apply a stereo modulated short delay line flanger after the Band-Reject summing node using 3rd-order Lagrange or Hermite fractional interpolation, a `90°` quadrature LFO phase offset between Left and Right channels, center delay time of `2.5 ms`, sweep depth of `±2.0 ms`, fixed internal feedback of `0.55`, sweep speed `flg_rate` (`0.01`–`20.0 Hz`, default `0.4 Hz`), wet/dry mix `flg_amount` (`0.0`–`1.0`), and `flg_bypass` defaulting to `1`.

#### Scenario: Stereo flanger modulation
- **WHEN** `flg_bypass` is `0` and `flg_amount` is increased above `0.0`
- **THEN** the 3rd-order interpolated delay sweeps `2.5 ms ± 2.0 ms` in stereo quadrature with `0.55` feedback

### Requirement: Band-Reject Summer Node
The system SHALL delay the auxiliary rejected band signal (`Low_Rejected + High_Rejected`) by the active Distortion oversampling latency and sum it with the output of Module 3 (`MM Filter`) scaled by `bp_br_mix` **before** feeding Module 4 (`Flanger`), following `Signal = MM_Filter_Out + (Delayed(Low_Rejected + High_Rejected) * bp_br_mix)`.

#### Scenario: Band-reject path recombining
- **WHEN** `bp_br_mix` is adjusted above `0.0` while 4x oversampled distortion is active
- **THEN** the rejected low and high bands are delayed by the exact oversampling group delay and summed immediately after the MM Filter without crossover comb filtering

### Requirement: Magic EQ Sub-Bass Boost
The system SHALL provide a Pultec-style resonant low-end enhancer (`Magic EQ`) featuring a resonant 2-pole peaking/low-shelf hybrid boost at `meq_tune` (`45.0`–`180.0 Hz`, default `80.0 Hz`, `0` to `+12 dB` scaled by `meq_amount`) paired with a complimentary dip at `2 * meq_tune` (`0` to `-2.5 dB`), a Phat Mode (`meq_phat = 1`) that places a `tanh` soft clipper inside the filter's resonant state feedback path and adds a `+3 dB` high-shelf air boost at `8 kHz` scaled by `meq_amount`, and a Pre-Distortion routing toggle (`meq_pre`) that switches the module between post-Flanger sub-bass enhancement (`meq_pre = 0`) and pre-Distortion resonant kick-tok injection (`meq_pre = 1`).

#### Scenario: Low frequency reinforcement
- **WHEN** `meq_amount` is increased, `meq_phat` is enabled (`1`), and `meq_pre` is `0`
- **THEN** sub-bass at `meq_tune` (`45–180 Hz`) is boosted after the Flanger with internal resonant `tanh` saturation, mud at `2 * meq_tune` is attenuated, and frequencies above `8 kHz` receive a `+3 dB` shelf boost

#### Scenario: Pre-Distortion resonant kick-tok injection
- **WHEN** `meq_pre` is enabled (`1`) and `meq_amount` is above `0.0`
- **THEN** `Magic EQ` processes the passband signal immediately before `Distortion` (and skips post-Flanger processing), driving the resonant peak at `meq_tune` directly into the 4-stage saturator to create pitched Hardstyle kick tails

### Requirement: Optical Compressor with Phat Mode
The system SHALL provide a feed-forward optical compressor supporting independent parameters for Threshold `comp_thresh` (`-48.0` to `0.0 dB`, default `-18.0 dB`), Ratio `comp_ratio` (`1.5:1` to `20.0:1`, default `4.0:1`), Attack `comp_attack` (`0.1` to `50.0 ms`, default `5.0 ms`), Release `comp_release` (`10.0` to `1500.0 ms`, default `120.0 ms`), and parallel Mix `comp_mix` (`0.0` to `1.0`, default `1.0`), alongside legacy `comp_amount` drive compatibility, operating with decibel-domain dual-stage optical release ballistics, dynamic auto-makeup compensation, and a selectable Phat Mode (`comp_phat = 1`) featuring a post-makeup 1st-order ADAA `tanh` soft-clip ceiling at `-0.1 dBFS`.

#### Scenario: Dynamic compression with Phat saturation
- **WHEN** `comp_thresh` is set below signal level and `comp_phat` is toggled between `0` and `1`
- **THEN** the compressor applies gain reduction based on `comp_thresh`, `comp_ratio`, `comp_attack`, and `comp_release`, switching between clean leveling and aggressive brickwall saturation with a `-0.1 dBFS` ADAA soft-clip ceiling

#### Scenario: Dual-stage optical release behavior
- **WHEN** a transient exceeds the compressor threshold and subsequently falls below threshold
- **THEN** the gain reduction decays in the decibel domain with a fast initial recovery stage (~50% of attenuation over 40–70 ms) followed by an exponential program-dependent tail stage scaled by `comp_release`

#### Scenario: Anti-aliased Phat saturation ceiling
- **WHEN** audio is processed in Phat Mode (`comp_phat = 1`) with peak levels exceeding the `-0.1 dBFS` ceiling
- **THEN** the saturation stage evaluates the 1st-order ADAA antiderivative of `tanh` across consecutive samples, suppressing digital foldback aliasing with zero latency

#### Scenario: Parallel compression blend
- **WHEN** `comp_mix` is adjusted between `0.0` and `1.0`
- **THEN** the output linearly crossfades between the uncompressed input audio and the compressed/saturated wet signal

### Requirement: Master Mix and Output Gain
The system SHALL delay the dry input signal by a buffer matching the active wet-chain oversampling latency, blend the latency-compensated dry signal with the wet chain via `master_mix` (`0.0`–`1.0`, default `1.0`), apply `master_vol` (`-36.0` to `+12.0 dB`, default `0.0 dB`) modulated by `ModTarget::MasterVol`, optionally sum sub-bass frequencies below `120.0 Hz` to mono when `master_mono_bass` (`1`) is enabled, optionally clamp output peaks to `0.0 dBFS` when `master_clip` (`1`, default `1`) is enabled, and support a click-free smoothed crossfaded global `master_bypass`.

#### Scenario: Dry/Wet balance control
- **WHEN** `master_mix` is blended at `0.5`
- **THEN** the dry path is delayed by the exact active oversampling group delay so combining dry and wet signals produces zero comb-filtering phase cancellation

#### Scenario: Sub-bass mono maker for club and festival compatibility
- **WHEN** `master_mono_bass` is enabled (`1`) on a stereo signal containing wide low-frequency modulation
- **THEN** frequencies below `120.0 Hz` are summed to mono (`0.5 * (L + R)`) while stereo width above `120.0 Hz` is preserved

#### Scenario: Master 0 dBFS output clipper and click-free bypass
- **WHEN** `master_clip` is enabled (`1`) or `master_bypass` is toggled during active audio playback
- **THEN** output peaks are limited to `0.0 dBFS` (`±1.0`) without `+18 dBFS` overshoots, and `master_bypass` crossfades smoothly between wet and latency-compensated dry signals without audible clicks

### Requirement: Per-Module Hardware-Free Bypass Toggles
Every DSP module in the processing chain SHALL provide an individual bypass toggle (`bp_bypass`, `dist_bypass`, `mm_bypass`, `flg_bypass`, `meq_bypass`, `comp_bypass`, and click-free crossfaded `master_bypass`) that routes the signal around that stage without allocations or clicks.

#### Scenario: Individual module bypass engagement
- **WHEN** a module's bypass parameter is set to `1`
- **THEN** that module leaves the signal unmodified while all other active modules in the topology continue processing normally

### Requirement: Dice Smart Parameter Randomizer
The system SHALL provide a smart randomizer trigger (`Dice`) that randomizes synthesis and effect parameters within musical bounds while keeping `master_vol` at `0.0 dB`, constraining `master_mix` between `0.4` and `1.0`, and preventing simultaneous extreme self-oscillation on both the BP and MM filters.

#### Scenario: User triggers Dice randomizer
- **WHEN** the Dice randomizer is triggered
- **THEN** effect parameters randomize to a new musical configuration with `master_vol` fixed at `0.0 dB`, `master_mix` within `[0.4, 1.0]`, and at most one filter bank exceeding `0.75` resonance

### Requirement: Stereo Input Signal Level Metering
The DSP engine SHALL provide lock-free atomic stereo peak and RMS level metering (`IMeterSource`) tapped immediately after the `input_gain` stage before the audio enters the BP filter splitter.

#### Scenario: Input level monitoring during playback
- **WHEN** audio is received at the plugin input
- **THEN** peak and RMS levels for left and right channels are computed after input gain and stored in lock-free atomic variables accessible to the UI at up to 60 FPS
