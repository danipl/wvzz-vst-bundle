## MODIFIED Requirements

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
