# Spec Delta

## ADDED Requirements

### Requirement: Modulation Target Profile for WvZz Reverb
The modulation module SHALL provide a `Reverb` target profile with 14 targets for LFO 1, LFO 2, and the X/Y pad: Pre-Delay, Size, Decay, High Damp, Low Damp, Diffusion, Tail Mod, Width, Shimmer, Reverb Mix, ER Level, Master Mix, LFO 2 Rate, and Master Vol. Existing profiles and their target order SHALL stay unchanged.

#### Scenario: LFO moves the reverb mix
- **WHEN** LFO 1 targets Reverb Mix with non-zero depth in WvZz Reverb
- **THEN** the modulation offset delivered for the Reverb Mix target is non-zero while the LFO runs

#### Scenario: Other profiles keep their lists
- **WHEN** any of the four other plugins registers its parameters
- **THEN** its target list has the same entries in the same order as before
