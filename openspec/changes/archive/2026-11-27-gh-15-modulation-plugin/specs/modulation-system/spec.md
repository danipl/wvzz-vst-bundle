# Spec Delta

## ADDED Requirements

### Requirement: Modulation Target Profile for WvZz Modulation
The modulation module SHALL provide a `Modulation` target profile with 14 targets for LFO 1, LFO 2, and the X/Y pad: Chorus Rate, Chorus Depth, Chorus Mix, Phaser Rate, Phaser Center, Phaser Feedback, Phaser Mix, Flanger Rate, Flanger Amount, Tremolo Rate, Tremolo Depth, Master Mix, LFO 2 Rate, and Master Vol. Existing profiles and their target order SHALL stay unchanged.

#### Scenario: LFO moves a chorus parameter
- **WHEN** LFO 1 targets Chorus Mix with non-zero depth in WvZz Modulation
- **THEN** the modulation offset delivered for the Chorus Mix target is non-zero while the LFO runs

#### Scenario: Existing profiles are unchanged
- **WHEN** WvZz Distortion or WvZz Dynamics registers its parameters
- **THEN** its target list has the same entries in the same order as before
