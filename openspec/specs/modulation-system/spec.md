# modulation-system Specification

## Purpose
Provides two control-rate low-frequency oscillators (LFO 1 and LFO 2) and an assignable 2D X/Y controller pad that modulate DSP parameters across the 21-item Modulatable Targets Enum with sample-accurate linear interpolation.

## Requirements

### Requirement: Control-Rate Dual LFO Generators
The system SHALL provide two independent modulation oscillators (LFO 1 and LFO 2) evaluated at a 32-sample control rate with modulated parameter offsets actively updated and propagated to target parameters, supporting 7 waveforms (`0: Sine`, `1: Triangle`, `2: Saw Down`, `3: Saw Up`, `4: Square`, `5: Sample & Hold`, `6: Smooth Random`).

#### Scenario: Control-rate LFO modulation with per-sample interpolation
- **WHEN** LFO 1 or LFO 2 is assigned to a target parameter with non-zero depth
- **THEN** the oscillator updates its control value every 32 samples and the active target offset buffer immediately reflects the new modulation value so that the target DSP parameter changes audibly without zipper noise

#### Scenario: Stepped and smooth random waveforms
- **WHEN** an LFO waveform is set to `5: Sample & Hold` or `6: Smooth Random`
- **THEN** `Sample & Hold` outputs white noise stepped at the LFO rate while `Smooth Random` outputs cubic-interpolated transitions between random values

### Requirement: LFO Free and Host Tempo Sync Timing
The system SHALL support both Free running rate (`0.02 Hz` to `30.0 Hz`) and Host Tempo Sync divisions (`4/1`, `2/1`, `1/1`, `1/2`, `1/4`, `1/8`, `1/16`, `1/32` including Dotted and Triplet variants) for LFO 1 and LFO 2, reading live host transport `bpm` and beat position (`ppqPosition`) from `ProcessContext` on every processing sub-block.

#### Scenario: Host tempo-synced LFO rate
- **WHEN** LFO sync mode is enabled and the host provides transport `bpm` (e.g., `128.0` or `150.0 BPM`) and `ppqPosition`
- **THEN** the LFO cycle frequency dynamically tracks the live host `bpm` (locked to `ppqPosition` when transport is playing) and falls back to free accumulation at `120.0 BPM` if host tempo is unavailable

### Requirement: 21-Target Modulation Routing Matrix
The system SHALL allow LFO 1, LFO 2, and the X/Y Pad axes to modulate any parameter in the active plugin's Modulatable Targets profile (22 Distortion targets for `WvZz Distortion` and 16 Dynamics targets for `WvZz Dynamics`).

#### Scenario: LFO 1 cross-modulating LFO 2 Rate
- **WHEN** LFO 1 target is set to `LFO 2 Rate` with a non-zero depth in either plugin profile
- **THEN** LFO 1's output dynamically shifts the instantaneous oscillation rate of LFO 2

#### Scenario: BP Both Cuts modulation
- **WHEN** a modulator targets `BP Both Cuts` in the Distortion profile
- **THEN** both `bp_low_cut` and `bp_high_cut` shift simultaneously in logarithmic frequency space

#### Scenario: X/Y Pad and LFO combined modulation
- **WHEN** both an LFO and an X/Y axis target the same parameter with non-zero depth
- **THEN** the modulation engine sums their offsets and supplies the combined offset to the audio processor

#### Scenario: Beat-synced Master Volume pumping and gating
- **WHEN** an LFO or X/Y axis targets `Master Vol` with non-zero depth
- **THEN** the master output gain is modulated in real time to produce rhythmic sidechain pumping, reverse-bass swells, or trance gating

#### Scenario: Dynamics target modulation
- **WHEN** an LFO or X/Y axis targets any Dynamics target (`Sub X-Over`, `SC HPF`, `Trans Attack`, `Trans Sustain`, `OTT Depth`, `OTT Upward`, `OTT Sheen`, `Comp Thresh`, `Comp Mix`, `Pump Amount`, `Pump Curve`, or `Clip Drive`) with non-zero depth
- **THEN** the target parameter is modulated in real time to produce dynamic transient movement, OTT breathing, or rhythmic pumping

### Requirement: Assignable Macro X/Y Controller Pad
The system SHALL provide a 2D macro controller (`xy_x` and `xy_y` in `0.0` to `1.0`) that applies additive bipolar or unipolar offsets (`xy_x_depth` and `xy_y_depth`) to any selected target from the Modulatable Targets Enum.

#### Scenario: X/Y pad macro offset application
- **WHEN** the user moves `xy_x` or `xy_y` with assigned targets and non-zero depth
- **THEN** the corresponding target parameters receive additive offsets proportional to the axis position and depth alongside any active LFO modulation

### Requirement: Modulation Target Profile for WvZz Modulation
The modulation module SHALL provide a `Modulation` target profile with 14 targets for LFO 1, LFO 2, and the X/Y pad: Chorus Rate, Chorus Depth, Chorus Mix, Phaser Rate, Phaser Center, Phaser Feedback, Phaser Mix, Flanger Rate, Flanger Amount, Tremolo Rate, Tremolo Depth, Master Mix, LFO 2 Rate, and Master Vol. Existing profiles and their target order SHALL stay unchanged.

#### Scenario: LFO moves a chorus parameter
- **WHEN** LFO 1 targets Chorus Mix with non-zero depth in WvZz Modulation
- **THEN** the modulation offset delivered for the Chorus Mix target is non-zero while the LFO runs

#### Scenario: Existing profiles are unchanged
- **WHEN** WvZz Distortion or WvZz Dynamics registers its parameters
- **THEN** its target list has the same entries in the same order as before

### Requirement: Modulation Target Profile for WvZz Delay
The modulation module SHALL provide a `Delay` target profile with 13 targets for LFO 1, LFO 2, and the X/Y pad: Delay Time, Feedback, LP Cutoff, HP Cutoff, Diffusion, Wow, Delay Mix, Grain Size, Grain Pitch, Grain Spray, Master Mix, LFO 2 Rate, and Master Vol. Existing profiles and their target order SHALL stay unchanged.

#### Scenario: LFO moves the delay feedback
- **WHEN** LFO 1 targets Feedback with non-zero depth in WvZz Delay
- **THEN** the modulation offset delivered for the Feedback target is non-zero while the LFO runs

#### Scenario: Other profiles keep their lists
- **WHEN** WvZz Distortion, WvZz Dynamics, or WvZz Modulation registers its parameters
- **THEN** its target list has the same entries in the same order as before

### Requirement: Modulation Target Profile for WvZz Reverb
The modulation module SHALL provide a `Reverb` target profile with 14 targets for LFO 1, LFO 2, and the X/Y pad: Pre-Delay, Size, Decay, High Damp, Low Damp, Diffusion, Tail Mod, Width, Shimmer, Reverb Mix, ER Level, Master Mix, LFO 2 Rate, and Master Vol. Existing profiles and their target order SHALL stay unchanged.

#### Scenario: LFO moves the reverb mix
- **WHEN** LFO 1 targets Reverb Mix with non-zero depth in WvZz Reverb
- **THEN** the modulation offset delivered for the Reverb Mix target is non-zero while the LFO runs

#### Scenario: Other profiles keep their lists
- **WHEN** any of the four other plugins registers its parameters
- **THEN** its target list has the same entries in the same order as before
