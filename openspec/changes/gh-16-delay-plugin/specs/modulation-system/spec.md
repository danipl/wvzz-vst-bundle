# Spec Delta

## ADDED Requirements

### Requirement: Modulation Target Profile for WvZz Delay
The modulation module SHALL provide a `Delay` target profile with 13 targets for LFO 1, LFO 2, and the X/Y pad: Delay Time, Feedback, LP Cutoff, HP Cutoff, Diffusion, Wow, Delay Mix, Grain Size, Grain Pitch, Grain Spray, Master Mix, LFO 2 Rate, and Master Vol. Existing profiles and their target order SHALL stay unchanged.

#### Scenario: LFO moves the delay feedback
- **WHEN** LFO 1 targets Feedback with non-zero depth in WvZz Delay
- **THEN** the modulation offset delivered for the Feedback target is non-zero while the LFO runs

#### Scenario: Other profiles keep their lists
- **WHEN** WvZz Distortion, WvZz Dynamics, or WvZz Modulation registers its parameters
- **THEN** its target list has the same entries in the same order as before
