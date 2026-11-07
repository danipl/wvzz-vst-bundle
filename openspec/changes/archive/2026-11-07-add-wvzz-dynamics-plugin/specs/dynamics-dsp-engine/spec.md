# dynamics-dsp-engine Specification

## Purpose
Defines the real-time zero-allocation audio processing pipeline for the WvZz Dynamics processor, encompassing the Linkwitz-Riley sub-bass crossover and sidechain detector filter, differential transient shaper and noise gate, 3-band upward/downward OTT processor, multi-topology bus compressor, dual-mode rhythmic pumper, and ADAA maximizer clipper.

## ADDED Requirements

### Requirement: Linkwitz-Riley Sub Isolator and Sidechain Detector Filter
The system SHALL provide a detector and sub-isolation stage (`det_*`) with a 24 dB/oct Linkwitz-Riley crossover (`det_sub_xover`: `40–300 Hz`), Sub Lock (`det_sub_lock`), Sub Tightener (`det_sub_tight`), Sidechain HPF (`det_sc_hp`: `20–500 Hz`), Sidechain Tilt (`det_sc_tilt`: `-6 to +6 dB`), and Sidechain Listen (`det_sc_listen`).

#### Scenario: Phase-coherent sub-bass isolation and recombination
- **WHEN** `det_sub_lock` is enabled (`1`) and `det_bypass` is `0`
- **THEN** frequencies below `det_sub_xover` are isolated into the auxiliary sub buffer, tightened according to `det_sub_tight`, and recombined phase-coherently before the Rhythmic Pumper and Maximizer Clipper stages so sub-bass energy does not trigger mid/high transient or OTT pumping

#### Scenario: Sidechain detector high-pass and spectral tilt filtering
- **WHEN** `det_sc_hp` is raised above `20.0 Hz` or `det_sc_tilt` is non-zero
- **THEN** the sidechain detector buffer fed to downstream dynamics detectors is high-passed at `det_sc_hp` and tilted by `det_sc_tilt` without filtering the audible main signal path (unless `det_sc_listen` is `1`)

#### Scenario: Sidechain listen auditioning
- **WHEN** `det_sc_listen` is enabled (`1`)
- **THEN** the plugin outputs the filtered sidechain detector signal directly so the user can audition the exact frequencies triggering compression and gating

### Requirement: Differential Transient Shaper and Noise Gate
The system SHALL provide a level-independent differential transient shaper and gate (`trans_*`) featuring fast/slow dual-envelope attack shaping (`trans_attack`: `-12 to +12 dB`), sustain shaping (`trans_sustain`: `-18 to +12 dB`), speed mode (`trans_speed`: `Fast` / `Punch`), and hysteresis noise gate (`trans_gate`: `-90 to -12 dB`).

#### Scenario: Positive transient attack boost for Hardstyle kick tok
- **WHEN** `trans_attack` is set above `0.0` with `trans_bypass = 0`
- **THEN** initial signal transients receive up to `+12.0 dB` of dynamic gain multiplication proportional to the positive differential between the fast attack and slow sustain envelopes, independent of absolute peak input level

#### Scenario: Sustain tail clamping and noise floor gating
- **WHEN** `trans_sustain` is negative or `trans_gate` is raised above the inter-note noise floor
- **THEN** post-transient decay tails are attenuated smoothly and signals falling below `trans_gate` are gated out so downstream upward compression does not amplify room or distortion hiss

### Requirement: 3-Band Upward and Downward OTT Processor
The system SHALL provide a 3-band (`Low < 120 Hz`, `Mid 120 Hz – 2.5 kHz`, `High > 2.5 kHz`) phase-aligned upward and downward compressor (`ott_*`) controlled by `ott_depth` (`0–1`), `ott_up_drive` (`0–1`), `ott_down_drive` (`0–1`), `ott_time` (`0.1–2.0`), and `ott_high_sheen` (`0 to +12 dB`).

#### Scenario: Upward compression of quiet harmonic tails
- **WHEN** `ott_depth` and `ott_up_drive` are above `0.0` and an input signal drops into the `-50.0` to `-20.0 dBFS` range
- **THEN** the 3-band OTT stage applies upward gain across the Low, Mid, and High bands to lift low-level harmonics and reverb air without clipping loud transients

#### Scenario: Euphoric High Sheen enhancement
- **WHEN** `ott_high_sheen` is increased above `0.0 dB` with `ott_bypass = 0`
- **THEN** the high band (`> 2.5 kHz`) receives pre-upward spectral emphasis and controlled downward ceiling clamping, producing dense supersaw and vocal sparkle

### Requirement: Dual-Mode Tempo and Sidechain Rhythmic Pumper
The system SHALL provide a rhythmic ducking module (`pump_*`) supporting Host Tempo-Synced PPQ envelopes and Audio/External Sidechain triggering (`pump_mode`), 9 sync divisions, 4 contour shapes (`Classic Duck`, `Hard Kick Clear`, `Reverse Bass`, `Festival Swell`), `pump_amount`, `pump_curve`, and `<180 Hz` `pump_sub_only`.

#### Scenario: Host tempo-synced 1/4-note pumping
- **WHEN** `pump_mode` is `0 (Tempo Sync)` and `pump_amount` is above `0.0`
- **THEN** the module attenuates gain rhythmically locked to `ProcessContext::hostPpqPosition` and `hostBpm` using the selected `pump_shape` and `pump_curve`

#### Scenario: External sidechain or audio transient triggered pumping
- **WHEN** `pump_mode` is `1 (Audio / Ext SC)` and sidechain audio exceeds `-24.0 dBFS`
- **THEN** the pumping envelope triggers from the sidechain detector input, ducking the signal on every incoming kick transient regardless of whether transport is playing

#### Scenario: Sub-only frequency ducking
- **WHEN** `pump_sub_only` is enabled (`1`) during active pumping
- **THEN** only frequencies below `180 Hz` are ducked by the rhythmic envelope while frequencies above `180 Hz` pass through un-ducked

### Requirement: Loudness Maximizer and Multi-Curve ADAA Clipper
The system SHALL provide a final brickwall loudness maximizer and anti-aliased clipper (`clip_*`) before the Master stage, featuring input drive `clip_drive` (`0 to +18 dB`), ceiling `clip_ceiling` (`-12 to 0 dBFS`), 3 selectable 1st-order ADAA curves (`Hard Clip`, `Warm Tube`, `Hardstyle Crunch`), and parallel `clip_mix`.

#### Scenario: Anti-aliased brickwall loudness maximization
- **WHEN** `clip_drive` boosts signal peaks above `clip_ceiling` with `clip_bypass = 0`
- **THEN** the stage boosts RMS loudness and clamps all output peaks strictly at or below `clip_ceiling` using 1st-order ADAA anti-aliased waveshaping across `Hard Clip`, `Warm Tube`, or `Hardstyle Crunch` modes
