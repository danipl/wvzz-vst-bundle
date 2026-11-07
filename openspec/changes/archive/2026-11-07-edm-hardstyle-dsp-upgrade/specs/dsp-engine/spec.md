## MODIFIED Requirements

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
