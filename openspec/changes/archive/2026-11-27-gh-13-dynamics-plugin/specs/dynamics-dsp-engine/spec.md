# Spec Delta

## MODIFIED Requirements

### Requirement: Differential Transient Shaper and Noise Gate
The system SHALL provide a level-independent differential transient shaper and gate (`trans_*`) featuring fast/slow dual-envelope attack shaping (`trans_attack`: `-1.0 to +1.0`, where `+1.0` gives about `+12 dB` of transient gain), sustain shaping (`trans_sustain`: `-1.0 to +1.0`, covering about `-18 dB` to `+12 dB` of tail gain), speed mode (`trans_speed`: `Fast` / `Punch`), and hysteresis noise gate (`trans_gate`: `-80 to 0 dB`, off at `-80 dB`).

#### Scenario: Positive transient attack boost for Hardstyle kick tok
- **WHEN** `trans_attack` is set above `0.0` with `trans_bypass = 0`
- **THEN** initial signal transients receive up to `+12.0 dB` of dynamic gain multiplication proportional to the positive differential between the fast attack and slow sustain envelopes, independent of absolute peak input level

#### Scenario: Sustain tail clamping and noise floor gating
- **WHEN** `trans_sustain` is negative or `trans_gate` is raised above the inter-note noise floor
- **THEN** post-transient decay tails are attenuated smoothly and signals falling below `trans_gate` are gated out so downstream upward compression does not amplify room or distortion hiss

#### Scenario: Gate is off at its minimum value
- **WHEN** `trans_gate` is at `-80 dB`
- **THEN** the noise gate passes all signal levels without attenuation

### Requirement: 3-Band Upward and Downward OTT Processor
The system SHALL provide a 3-band (`Low < 120 Hz`, `Mid 120 Hz – 2.5 kHz`, `High > 2.5 kHz`) phase-aligned upward and downward compressor (`ott_*`) controlled by `ott_depth` (`0–1`), `ott_up_drive` (`0–1`), `ott_down_drive` (`0–1`), `ott_time` (`0.1–2.0`), and `ott_sheen` (`0 to +12 dB`).

#### Scenario: Upward compression of quiet harmonic tails
- **WHEN** `ott_depth` and `ott_up_drive` are above `0.0` and an input signal drops into the `-50.0` to `-20.0 dBFS` range
- **THEN** the 3-band OTT stage applies upward gain across the Low, Mid, and High bands to lift low-level harmonics and reverb air without clipping loud transients

#### Scenario: Euphoric High Sheen enhancement
- **WHEN** `ott_sheen` is increased above `0.0 dB` with `ott_bypass = 0`
- **THEN** the high band (`> 2.5 kHz`) receives pre-upward spectral emphasis and controlled downward ceiling clamping, producing dense supersaw and vocal sparkle
