# Spec Delta

## MODIFIED Requirements

### Requirement: Control-Rate Dual LFO Generators
The system SHALL provide two independent modulation oscillators (LFO 1 and LFO 2) evaluated at a 32-sample control rate with modulated parameter offsets actively updated and propagated to target parameters, supporting 7 waveforms (`0: Sine`, `1: Triangle`, `2: Saw Down`, `3: Saw Up`, `4: Square`, `5: Sample & Hold`, `6: Smooth Random`).

#### Scenario: Control-rate LFO modulation with per-sample interpolation
- **WHEN** LFO 1 or LFO 2 is assigned to a target parameter with non-zero depth
- **THEN** the oscillator updates its control value every 32 samples and the active target offset buffer immediately reflects the new modulation value so that the target DSP parameter changes audibly without zipper noise

#### Scenario: Stepped and smooth random waveforms
- **WHEN** an LFO waveform is set to `5: Sample & Hold` or `6: Smooth Random`
- **THEN** `Sample & Hold` outputs white noise stepped at the LFO rate while `Smooth Random` outputs cubic-interpolated transitions between random values

### Requirement: 21-Target Modulation Routing Matrix
The system SHALL allow LFO 1, LFO 2, and the X/Y Pad axes to modulate any parameter in the Modulatable Targets Enum (`None`, `BP Low Cut`, `BP High Cut`, `BP Both Cuts`, `BP Low Res`, `BP High Res`, `BP BR Mix`, `Dist Mech`, `Dist Tube`, `Dist Bit`, `Dist Xcita`, `MM Cutoff`, `MM Res`, `MM Mix`, `Flanger Rate`, `Flanger Amount`, `Magic EQ Amount`, `Magic EQ Tune`, `Comp Amount`, `Master Mix`, and `LFO 2 Rate` for LFO 1 only).

#### Scenario: LFO 1 cross-modulating LFO 2 Rate
- **WHEN** LFO 1 target is set to `LFO 2 Rate` with a non-zero depth
- **THEN** LFO 1's output dynamically shifts the instantaneous oscillation rate of LFO 2

#### Scenario: BP Both Cuts modulation
- **WHEN** a modulator targets `BP Both Cuts`
- **THEN** both `bp_low_cut` and `bp_high_cut` shift simultaneously in logarithmic frequency space

#### Scenario: X/Y Pad and LFO combined modulation
- **WHEN** both an LFO and an X/Y axis target the same parameter with non-zero depth
- **THEN** the modulation engine sums their offsets and supplies the combined offset to the audio processor
