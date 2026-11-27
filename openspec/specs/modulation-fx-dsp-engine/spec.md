# modulation-fx-dsp-engine Specification

## Purpose

Defines the Chorus, Phaser, and Tremolo effects and the fixed serial chain, bypass behavior, and factory presets of the WvZz Modulation plugin.

## Requirements

### Requirement: Multi-Voice Stereo Chorus
The system SHALL provide a chorus (`chor_*`) with 1 to 4 modulated delay voices (`chor_voices`), sweep speed `chor_rate` (`0.05–10 Hz`, default `0.8`), sweep depth `chor_depth` (`0–1`, up to `±5 ms`), base delay `chor_delay` (`2–30 ms`, default `12`), stereo spread `chor_spread` (`0–1`), feedback `chor_feedback` (`0–0.7`, default `0`), wet/dry `chor_mix` (`0–1`, default `0.5`), and `chor_bypass` (default `1`).

#### Scenario: Chorus adds modulated voices
- **WHEN** `chor_bypass` is `0`, `chor_mix` is `0.5`, and a steady sine tone is processed
- **THEN** the output differs from the input and its level changes over time because the delayed voices drift against the dry tone

#### Scenario: Stereo spread decorrelates the channels
- **WHEN** `chor_spread` is `1.0` and a mono-compatible input (same signal on left and right) is processed
- **THEN** the left and right outputs differ from each other
- **AND** with `chor_spread` at `0.0` the left and right outputs are equal

#### Scenario: Chorus mix at zero is dry
- **WHEN** `chor_bypass` is `0` and `chor_mix` is `0.0`
- **THEN** the output equals the input

### Requirement: All-Pass Cascade Phaser with Feedback
The system SHALL provide a phaser (`phs_*`) built from a cascade of first-order all-pass stages (`phs_stages`: `2`, `4`, `6`, `8`, or `12`, default `4`) swept around `phs_center` (`100–4000 Hz`, default `800`) by an LFO with `phs_rate` (`0.02–10 Hz`, default `0.5`) and `phs_depth` (`0–1`, up to `±2` octaves), with `phs_feedback` (`-0.95 to 0.95`, default `0.5`), left/right LFO offset `phs_stereo` (`0–1`, `1` = `180°`), `phs_mix` (`0–1`, default `0.5`), and `phs_bypass` (default `1`).

#### Scenario: Phaser creates moving notches
- **WHEN** `phs_bypass` is `0`, `phs_mix` is `0.5`, and broadband noise is processed
- **THEN** the output spectrum shows notches whose frequency moves with the LFO, so the output differs from the input

#### Scenario: Phaser stays stable at maximum feedback
- **WHEN** `phs_feedback` is `0.95` or `-0.95` with `12` stages and a full-scale input is processed for 2 seconds
- **THEN** every output sample is finite and its absolute value stays below `8.0`

#### Scenario: Stage count changes the response
- **WHEN** the same input is processed with `phs_stages` at `2` and at `12`
- **THEN** the two outputs differ

### Requirement: Classic and Harmonic Tremolo
The system SHALL provide a tremolo (`trem_*`) with `trem_rate` (`0.1–20 Hz`, default `4`), `trem_depth` (`0–1`, default `0.5`), LFO `trem_shape` (`Sine`, `Triangle`, `Square`), `trem_mode` (`Classic`, `Harmonic`), left/right LFO offset `trem_stereo` (`0–1`, `1` = `180°`), harmonic crossover `trem_xover` (`200–2000 Hz`, default `800`), and `trem_bypass` (default `1`).

#### Scenario: Classic tremolo modulates the level
- **WHEN** `trem_mode` is `Classic`, `trem_depth` is `1.0`, and a constant-level tone is processed
- **THEN** the output level moves between about the input level and near silence at `trem_rate`

#### Scenario: Harmonic tremolo moves low and high bands in opposite phase
- **WHEN** `trem_mode` is `Harmonic` and `trem_depth` is `1.0`
- **THEN** the band below `trem_xover` is loudest at the moment the band above `trem_xover` is quietest, and the reverse half a cycle later

#### Scenario: Zero depth is transparent
- **WHEN** `trem_depth` is `0.0` in either mode
- **THEN** the output equals the input within `-80 dB`

#### Scenario: Full stereo offset pans the signal
- **WHEN** `trem_stereo` is `1.0` in `Classic` mode with `trem_depth` at `1.0`
- **THEN** the left channel is loudest when the right channel is quietest

### Requirement: Fixed Serial Modulation Chain
The WvZz Modulation plugin SHALL process audio in the fixed order Chorus → Phaser → Flanger → Tremolo → Master, where each effect has its own bypass and the Master module provides input gain, wet/dry mix, output volume, and global bypass.

#### Scenario: All effects bypassed
- **WHEN** `chor_bypass`, `phs_bypass`, `flg_bypass`, and `trem_bypass` are all `1` and the Master controls are at their defaults
- **THEN** the output equals the input

#### Scenario: Effects stack in series
- **WHEN** Chorus and Tremolo are both active
- **THEN** the tremolo level modulation is applied to the chorused signal

### Requirement: Real-Time Safety of Modulation Effects
The Chorus, Phaser, Tremolo, and Flanger modules SHALL allocate their delay buffers only in `prepare`, perform no heap allocation and take no lock while processing audio, and keep feedback paths free of denormal numbers.

#### Scenario: Zero allocations while processing
- **WHEN** each module processes 1,000 blocks after `prepare`
- **THEN** the allocation counter of the test harness reports `0` heap allocations

#### Scenario: Parameter ids follow the instance prefix
- **WHEN** a module is created with a custom prefix such as `slot2_chor`
- **THEN** all of its parameter ids start with `slot2_chor_`

### Requirement: WvZz Modulation Factory Presets
The WvZz Modulation plugin SHALL expose 10 factory presets through the host program interface, and loading a preset SHALL set every effect parameter, so the result does not depend on the preset loaded before.

#### Scenario: All presets load and stay stable
- **WHEN** each of the 10 presets is selected and audio is processed
- **THEN** the program name matches the preset name and the output contains no NaN or infinite sample

#### Scenario: Preset result is independent of history
- **WHEN** preset A is loaded directly, and again after loading preset B first
- **THEN** all effect parameter values are identical in both cases
