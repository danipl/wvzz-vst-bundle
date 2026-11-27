# Spec Delta

## ADDED Requirements

### Requirement: Sub-Bass Mono Maker Acts on the Stereo Difference
When `master_mono_bass` is on, the Master module SHALL remove the left/right difference below `120 Hz` and SHALL leave signal that is equal on both channels unchanged. Stereo width above the crossover SHALL be kept.

#### Scenario: Out-of-phase lows cancel
- **WHEN** `master_mono_bass` is on and a `60 Hz` tone plays with opposite polarity on left and right
- **THEN** the output level is below `15%` of the input level on both channels

#### Scenario: Mono content is unchanged
- **WHEN** `master_mono_bass` is on and the same `60 Hz` tone plays on left and right
- **THEN** the output equals the input

#### Scenario: Highs keep their width
- **WHEN** `master_mono_bass` is on and a `2 kHz` tone plays with opposite polarity on left and right
- **THEN** the output level stays above `95%` of the input level on both channels

#### Scenario: Switch off is transparent
- **WHEN** `master_mono_bass` is off
- **THEN** the stage does not change the signal
