# Spec Delta

## ADDED Requirements

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

## MODIFIED Requirements

### Requirement: Band-Pass Splitter Routing
The DSP engine SHALL split incoming stereo audio into a Passband signal and a Rejected Band signal using two 12 dB/oct (2-pole) Cytomic zero-delay feedback (ZDF) trapezoidal State Variable Filters (one High-Pass at `bp_low_cut` with `bp_low_res`, one Low-Pass at `bp_high_cut` with `bp_high_res`, Q mapped `0.707` to `12.0`), where `Low_Rejected = LowPass_Output(Input, bp_low_cut, bp_low_res)`, `High_Rejected = HighPass_Output(Input, bp_high_cut, bp_high_res)`, `Passband = BandPass_Cascade(Input)`, and `bp_link` locks the logarithmic frequency delta between `bp_low_cut` and `bp_high_cut`.

#### Scenario: Audio band splitting
- **WHEN** audio is processed through the BP Filter splitter with `bp_bypass = 0`
- **THEN** frequencies within the passband cascade pass to the Distortion stage while `Low_Rejected + High_Rejected` are extracted for post-MM-Filter recombination

#### Scenario: Linked cutoff movement
- **WHEN** `bp_link` is enabled (`1`) and either `bp_low_cut` or `bp_high_cut` is adjusted
- **THEN** the companion cutoff updates automatically to preserve the logarithmic frequency ratio within `[20.0 Hz, 20000.0 Hz]`

### Requirement: 4-Stage Oversampled Distortion
The system SHALL process the passband audio through 4 distortion circuits (`Mech`, `Tube`, `Bitcrusher`, and `Xcita`) where `Bitcrusher` applies amplitude quantization (`16-bit` to `4-bit`) and sample-and-hold decimation (`1x` to `32x`) at the base sample rate before upsampling, and `Mech` (`1.2 kHz` +6 dB Q=1.0 pre-emphasis, asymmetric transfer `(x >= 0) ? tanh(drive * x) + 0.15 * sin(drive * x * pi) : clamp(drive * 1.2 * x, -0.9, 0.0)`, post-de-emphasis, exp drive `1.0`–`25.0`), `Tube` (`tanh(drive * x + 0.1 * x^2) - tanh(0.1 * x^2)` with DC blocker, exp drive `1.0`–`15.0`), and `Xcita` (`2.8 kHz` 2-pole Butterworth HPF, `tanh(drive * (0.7 * x_hf + 0.3 * abs(x_hf)))`, `10 Hz` DC blocker) run inside a 4x polyphase IIR/FIR oversampling block.

#### Scenario: Parallel distortion saturation
- **WHEN** distortion amounts for `dist_mech`, `dist_tube`, `dist_bit`, or `dist_xcita` are increased above zero with `dist_bypass = 0`
- **THEN** each active circuit generates its specified harmonic transfer curve and DC-blocked output without aliasing or DC bias

### Requirement: Modulated Multi-Mode Filter
The system SHALL provide a multi-mode filter (`MM Filter`) after the Distortion stage supporting all 12 topologies (`0: LP 12dB`, `1: LP 24dB` with internal `tanh` feedback saturation, `2: BP 12dB`, `3: BP 24dB`, `4: HP 12dB`, `5: HP 24dB`, `6: Notch`, `7: Peak`, `8: Comb +`, `9: Comb -`, `10: RingMod`, `11: Vowel / Formant` morphing across `A-E-I-O-U` formant tables via `mm_cutoff`), with wet/dry blend (`mm_mix`) and cutoff modulation (`mm_env_depth` in `±6 octaves`) driven by an Envelope Follower (`mm_env_att` `0.1`–`1000 ms`, `mm_env_rel` `1.0`–`5000 ms`) tapped from the **Plugin Input (pre-BP Filter)**.

#### Scenario: Envelope follower cutoff modulation
- **WHEN** audio enters the plugin input and `mm_env_depth` is non-zero
- **THEN** the envelope follower tracks the pre-BP input level using decoupled attack/release ballistics and shifts `mm_cutoff` across a `±6 octave` range regardless of distortion gain changes

#### Scenario: 12-topology filter selection
- **WHEN** `mm_type` is switched to any index from `0` to `11`
- **THEN** the MM Filter processes audio using the selected SVF, Ladder, Comb, RingMod, or Formant vowel topology blended by `mm_mix`

### Requirement: Band-Reject Summer Node
The system SHALL sum the output of Module 3 (`MM Filter`) with the rejected band signal (`(Low_Rejected + High_Rejected) * bp_br_mix`) **before** feeding Module 4 (`Flanger`), following `Signal = MM_Filter_Out + ((Low_Rejected + High_Rejected) * bp_br_mix)`.

#### Scenario: Band-reject path recombining
- **WHEN** `bp_br_mix` is adjusted above `0.0`
- **THEN** the rejected low and high bands are summed back immediately after the MM Filter and the combined signal feeds into the Flanger module

### Requirement: Stereo Modulated Flanger
The system SHALL apply a stereo modulated short delay line flanger after the Band-Reject summing node using 3rd-order Lagrange or Hermite fractional interpolation, a `90°` quadrature LFO phase offset between Left and Right channels, center delay time of `2.5 ms`, sweep depth of `±2.0 ms`, fixed internal feedback of `0.55`, sweep speed `flg_rate` (`0.01`–`20.0 Hz`, default `0.4 Hz`), wet/dry mix `flg_amount` (`0.0`–`1.0`), and `flg_bypass` defaulting to `1`.

#### Scenario: Stereo flanger modulation
- **WHEN** `flg_bypass` is `0` and `flg_amount` is increased above `0.0`
- **THEN** the 3rd-order interpolated delay sweeps `2.5 ms ± 2.0 ms` in stereo quadrature with `0.55` feedback

### Requirement: Magic EQ Sub-Bass Boost
The system SHALL provide a Pultec-style resonant low-end enhancer (`Magic EQ`) featuring a resonant 2-pole peaking/low-shelf hybrid boost at `meq_tune` (`60.0`–`120.0 Hz`, default `80.0 Hz`, `0` to `+12 dB` scaled by `meq_amount`) paired with a complimentary dip at `2 * meq_tune` (`0` to `-2.5 dB`), and a Phat Mode (`meq_phat = 1`) that places a `tanh` soft clipper inside the filter's resonant state feedback path and adds a `+3 dB` high-shelf air boost at `8 kHz` scaled by `meq_amount`.

#### Scenario: Low frequency reinforcement
- **WHEN** `meq_amount` is increased and `meq_phat` is enabled (`1`)
- **THEN** sub-bass at `meq_tune` is boosted with internal resonant `tanh` saturation, mud at `2 * meq_tune` is attenuated, and frequencies above `8 kHz` receive a gentle `+3 dB` shelf boost

### Requirement: Optical Compressor with Phat Mode
The system SHALL provide a one-knob feed-forward compressor where `comp_amount` (`0.0`–`1.0`) simultaneously drives Threshold (`0 dB` down to `-36 dB`) and calculated Auto-Makeup Gain (`0 dB` to `+14 dB`) with adjustable release `comp_release` (`10.0`–`1500.0 ms`, default `120.0 ms`), operating in Normal Mode (`comp_phat = 0`: Attack `3.5 ms`, Ratio `3.5:1`, Soft Knee `6 dB`) or Phat Mode (`comp_phat = 1`: Attack `0.8 ms`, Ratio `12:1`, Hard Knee `1 dB`, plus a post-makeup `tanh` soft-clip ceiling at `-0.1 dBFS`).

#### Scenario: Dynamic compression with Phat saturation
- **WHEN** `comp_amount` is increased and `comp_phat` is toggled between `0` and `1`
- **THEN** the compressor switches between smooth `3.5:1` leveling (`3.5 ms` attack, `6 dB` knee) and aggressive `12:1` brickwall smashing (`0.8 ms` attack, `1 dB` knee, `-0.1 dBFS` soft-clip ceiling) with automatic makeup gain

### Requirement: Master Mix and Output Gain
The system SHALL delay the dry input signal by a buffer matching the Distortion oversampler's group delay, blend the latency-compensated dry signal with the wet chain via `master_mix` (`0.0`–`1.0`, default `1.0`), apply `master_vol` (`-36.0` to `+12.0 dB`, default `0.0 dB`), and support a click-free crossfaded global `master_bypass`.

#### Scenario: Dry/Wet balance control
- **WHEN** `master_mix` is blended at `0.5`
- **THEN** the dry path is delayed by the exact oversampling group delay so combining dry and wet signals produces zero comb-filtering phase cancellation
