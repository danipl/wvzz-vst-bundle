# reverb-dsp-engine Specification

## Purpose

Defines the early-reflections stage, the 8-line Feedback Delay Network reverb with its Plate, Hall, and Shimmer modes, and the chain, bypass behavior, and factory presets of the WvZz Reverb plugin.

## Requirements

### Requirement: Early Reflections Stage
The EarlyReflections module (`er_*`) SHALL create a stereo pattern of discrete early echoes from the input, with pattern length `er_size` (`0–1`, about `16–80 ms`), all-pass smearing `er_diffusion` (`0–1`), output level `er_level` (`0–1`, default `0.5`), and `er_bypass`. It SHALL write its result to the auxiliary bus so the main reverb can use it as extra input and as part of the wet signal.

#### Scenario: Impulse becomes a pattern of echoes
- **WHEN** one impulse is processed with `er_level` above `0`
- **THEN** the auxiliary bus holds at least 8 separate echoes per channel within the pattern length, and the left and right patterns differ

#### Scenario: Size scales the pattern
- **WHEN** `er_size` is `1.0` instead of `0.0`
- **THEN** the last echo arrives later

#### Scenario: Level zero or bypass gives no early reflections
- **WHEN** `er_level` is `0.0` or `er_bypass` is on
- **THEN** the module adds nothing to the auxiliary bus

### Requirement: Eight-Line Feedback Delay Network
The Reverb module (`rev_*`) SHALL build its tail from 8 delay lines connected through an energy-preserving mixing matrix, with one gain per line computed from the line length and `rev_decay`, so the loop gain stays below 1 at every setting.

#### Scenario: Mixing matrix preserves energy
- **WHEN** any 8-value vector passes through the mixing matrix
- **THEN** the sum of squares of the output equals the sum of squares of the input

#### Scenario: Output stays bounded at maximum decay and shimmer
- **WHEN** `rev_decay` is at its maximum, `rev_shimmer` is `1.0`, and a full-scale signal is processed for 30 seconds
- **THEN** every output sample is finite, its absolute value stays below `8.0`, and the level does not keep growing

### Requirement: Decay Time Control
`rev_decay` (`0.2–20 s`, default `2.5`) SHALL set the time in which the tail falls by `60 dB` at mid frequencies. `rev_size` (`0–1`) SHALL scale the delay line lengths without changing that decay time.

#### Scenario: Measured decay matches the setting
- **WHEN** a noise burst is processed in `Plate` or `Hall` mode with the damping filters fully open
- **THEN** the decay time measured from the slope of the tail level is within `10%` of `rev_decay`

#### Scenario: Size keeps the decay time
- **WHEN** `rev_size` changes from `0.2` to `0.9` with the same `rev_decay`
- **THEN** the measured decay time is still within `10%` of `rev_decay`

### Requirement: Reverb Modes
The Reverb module SHALL provide `rev_mode` with `Plate` (short delay lines, dense and bright), `Hall` (long delay lines, slower build-up), and `Shimmer` (Hall with a pitch-shifted copy of the tail fed back into the network).

#### Scenario: Hall builds up more slowly than Plate
- **WHEN** the same impulse is processed in `Plate` and `Hall` mode with the same size
- **THEN** the Hall tail reaches its echo density later than the Plate tail

#### Scenario: Shimmer adds pitch-shifted energy
- **WHEN** `rev_mode` is `Shimmer`, `rev_shimmer` is `1.0`, `rev_interval` is one octave up, and a `220 Hz` tone burst is processed
- **THEN** the tail holds much more energy at `440 Hz` than with `rev_shimmer` at `0.0`

### Requirement: Pre-Delay, Damping, Diffusion, Modulation, and Width
The Reverb module SHALL provide `rev_predelay` (`0–250 ms`), high damping `rev_hidamp` (`1000–20000 Hz`: highs above it decay faster), low damping `rev_lodamp` (`20–1000 Hz`: lows below it decay faster), input diffusion `rev_diffusion` (`0–1`), tail modulation `rev_mod` (`0–1`), and stereo `rev_width` (`0–1`).

#### Scenario: Pre-delay delays the tail
- **WHEN** `rev_predelay` is `100 ms` and one impulse is processed without early reflections
- **THEN** the wet signal is silent for the first `100 ms`

#### Scenario: High damping darkens the tail
- **WHEN** `rev_hidamp` is `1000 Hz` instead of `20000 Hz`
- **THEN** the tail has less high-frequency energy

#### Scenario: Width zero is mono
- **WHEN** `rev_width` is `0.0`
- **THEN** the left and right wet signals are equal, and with `rev_width` at `1.0` they differ

### Requirement: Mix and Bypass with Tail Fade-Out
`rev_mix` (`0–1`, default `0.3`) SHALL blend dry and wet. When `rev_bypass` is turned on, the module SHALL stop feeding the network, pass the dry signal at full level, and let the existing tail decay.

#### Scenario: Mix at zero is dry
- **WHEN** `rev_mix` is `0.0`
- **THEN** the output equals the input

#### Scenario: Tail continues after bypass
- **WHEN** a tail is sounding and `rev_bypass` is turned on
- **THEN** the tail keeps decaying and audio played after the bypass creates no new reverb

### Requirement: Real-Time Safety of Reverb Modules
The EarlyReflections and Reverb modules SHALL allocate memory only in `prepare`, take no lock while processing, flush denormal numbers in every feedback path, and use the instance prefix for every parameter id.

#### Scenario: Zero allocations while processing
- **WHEN** each module processes 1,000 blocks after `prepare`, in every mode
- **THEN** the allocation counter of the test harness reports `0`

#### Scenario: Parameter ids follow the instance prefix
- **WHEN** a module is created with a custom prefix such as `slot2_rev`
- **THEN** all of its parameter ids start with `slot2_rev_`

### Requirement: WvZz Reverb Chain and Factory Presets
The WvZz Reverb plugin SHALL process audio in the order EarlyReflections → Reverb → Master and SHALL expose 10 factory presets whose result does not depend on the preset loaded before.

#### Scenario: All presets load and stay stable
- **WHEN** each of the 10 presets is selected and audio is processed
- **THEN** the program name matches the preset name and the output contains no NaN or infinite sample

#### Scenario: Preset result is independent of history
- **WHEN** a preset is loaded directly, and again after another preset
- **THEN** all parameter values are identical in both cases
