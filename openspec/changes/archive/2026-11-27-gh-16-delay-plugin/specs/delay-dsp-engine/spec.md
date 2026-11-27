# Spec Delta

## Purpose

Defines the reusable fractional delay line, the Delay module with its Clean, Tape, BBD, and Granular modes, the grain engine, and the chain, bypass behavior, and factory presets of the WvZz Delay plugin.

## ADDED Requirements

### Requirement: Delay Time and Tempo Sync
The Delay module (`dly_*`) SHALL delay the signal by `dly_time` (`20–4000 ms`, default `350`), or, when `dly_sync` is on, by the note value `dly_sync_div` at the host tempo, limited to `4000 ms`. The right channel SHALL add `dly_offset` (`0–50 ms`). All delay buffers SHALL be sized for 4 seconds in `prepare`.

#### Scenario: Impulse arrives after the set time
- **WHEN** `dly_mode` is `Clean`, `dly_time` is `100 ms`, feedback is `0`, and one impulse is processed at `48 kHz`
- **THEN** the wet impulse appears `4800` samples after the input impulse, within `1` sample

#### Scenario: Tempo-synced time
- **WHEN** `dly_sync` is on, `dly_sync_div` is `1/4`, and the host tempo is `120 BPM`
- **THEN** the delay time is `500 ms`

#### Scenario: Maximum time
- **WHEN** `dly_time` is `4000 ms`
- **THEN** the wet impulse appears 4 seconds after the input impulse and no buffer is resized while processing

### Requirement: Stereo and Ping-Pong Routing
The Delay module SHALL offer `dly_routing` with `Stereo` (left and right repeat independently) and `Ping-Pong` (the input is summed to mono and the repeats alternate between left and right).

#### Scenario: Ping-pong alternates sides
- **WHEN** `dly_routing` is `Ping-Pong`, feedback is above `0`, and one impulse is processed
- **THEN** the first repeat is only on the left, the second only on the right, and the third on the left again

### Requirement: Filtered Feedback with Soft Clipper
The feedback loop SHALL apply `dly_feedback` (`0–1.1`, default `0.4`), a high-pass `dly_hp` (`20–2000 Hz`), a low-pass `dly_lp` (`500–20000 Hz`), and a soft clipper, so the output stays bounded at any setting.

#### Scenario: Output stays bounded above 100% feedback
- **WHEN** `dly_feedback` is `1.1` and a full-scale signal is processed for 30 seconds in each mode
- **THEN** every output sample is finite and its absolute value stays below `8.0`

#### Scenario: Low-pass darkens later repeats
- **WHEN** `dly_lp` is `1000 Hz` and broadband noise bursts are processed with feedback above `0`
- **THEN** the third repeat has less energy above `4 kHz` than the first repeat

### Requirement: Delay Character Modes
The Delay module SHALL provide `dly_mode` with `Clean` (transparent), `Tape` (saturation from `dly_drive` plus wow and flutter from `dly_wow`), `BBD` (a dark low-pass in the loop that gets darker with longer times), and `Granular` (the repeats are rebuilt from grains by the grain engine).

#### Scenario: Clean and Granular time change crossfades
- **WHEN** the delay time changes while a steady low tone plays in `Clean` or `Granular` mode
- **THEN** the old and new delay positions crossfade over about `50 ms` (or one grain length) and the output has no sample-to-sample jump larger than a click threshold

#### Scenario: Tape and BBD time change bends the pitch
- **WHEN** the delay time changes while a steady tone plays in `Tape` or `BBD` mode
- **THEN** the delay time glides to the new value and the pitch of the repeats moves away from the input pitch during the glide

#### Scenario: BBD is darker than Clean
- **WHEN** the same broadband signal is processed in `BBD` and in `Clean` mode
- **THEN** the BBD repeats have less energy above `6 kHz`

#### Scenario: Tape wow modulates the time
- **WHEN** `dly_mode` is `Tape` and `dly_wow` is `1.0`
- **THEN** the repeats of a steady tone vary in pitch over time, and with `dly_wow` at `0.0` they do not

### Requirement: Diffusion
The Delay module SHALL smear the repeats with all-pass diffusion controlled by `dly_diffusion` (`0–1`, default `0`). At `0` the repeats are unchanged.

#### Scenario: Diffusion spreads an impulse
- **WHEN** `dly_diffusion` is `1.0` and one impulse is processed
- **THEN** the first repeat is spread over many samples instead of one, and with `dly_diffusion` at `0.0` it is a single impulse at the set time

### Requirement: Ducking
The Delay module SHALL lower the wet level by up to `dly_duck` (`0–1`) while the dry input is present and SHALL bring it back after the input stops.

#### Scenario: Wet level drops under the dry signal
- **WHEN** `dly_duck` is `1.0` and a loud dry signal plays
- **THEN** the wet level is lower than with `dly_duck` at `0.0`, and after the dry signal stops the repeats return to full level

### Requirement: Bypass with Tail Fade-Out
When `dly_bypass` is turned on, the Delay module SHALL stop sending new input into the delay, pass the dry signal at full level, and keep playing the repeats already in the delay until they decay.

#### Scenario: Tail continues after bypass
- **WHEN** repeats are sounding and `dly_bypass` is turned on
- **THEN** the existing repeats continue and decay, and audio played after the bypass creates no new repeats

#### Scenario: Bypassed and empty is transparent
- **WHEN** `dly_bypass` is on and the delay holds no audio
- **THEN** the output equals the input

### Requirement: Grain Engine with 32 Voices and Pitch Shift
The Grain module (`grain_*`) SHALL replay delayed audio as overlapping windowed grains using at most 32 pre-allocated voices, with `grain_size` (`10–500 ms`, default `80`), `grain_density` (`2–60` grains per second, default `20`), `grain_pitch` (`-24 to +24` semitones), `grain_spray` (`0–1` random position), `grain_reverse` (`0–1` chance of reversed grains), and `grain_spread` (`0–1` stereo scatter).

#### Scenario: Pitch shift up one octave
- **WHEN** `grain_pitch` is `+12` and a steady `220 Hz` tone is processed
- **THEN** the grain output has its main energy near `440 Hz`

#### Scenario: Voice count is limited
- **WHEN** `grain_size` and `grain_density` ask for more than 32 overlapping grains
- **THEN** the number of active voices never exceeds 32 and the output stays finite

#### Scenario: Granular delay keeps the delay time
- **WHEN** `dly_mode` is `Granular` with `grain_pitch` at `0` and `grain_spray` at `0`
- **THEN** the repeats of a short burst are centered at the set delay time

### Requirement: Real-Time Safety of Delay and Grain
The Delay and Grain modules SHALL allocate memory only in `prepare`, take no lock while processing, flush denormal numbers in every feedback path, and use the instance prefix for every parameter id.

#### Scenario: Zero allocations while processing
- **WHEN** each module processes 1,000 blocks after `prepare`, in every mode
- **THEN** the allocation counter of the test harness reports `0`

#### Scenario: Parameter ids follow the instance prefix
- **WHEN** a module is created with a custom prefix such as `slot2_dly`
- **THEN** all of its parameter ids start with `slot2_dly_`

### Requirement: WvZz Delay Chain and Factory Presets
The WvZz Delay plugin SHALL process audio in the order Delay (using the Grain module in Granular mode) → Master, and SHALL expose 10 factory presets whose result does not depend on the preset loaded before.

#### Scenario: All presets load and stay stable
- **WHEN** each of the 10 presets is selected and audio is processed
- **THEN** the program name matches the preset name and the output contains no NaN or infinite sample

#### Scenario: Preset result is independent of history
- **WHEN** a preset is loaded directly, and again after another preset
- **THEN** all parameter values are identical in both cases
