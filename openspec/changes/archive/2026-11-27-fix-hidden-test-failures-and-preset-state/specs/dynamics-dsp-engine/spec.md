# Spec Delta

## ADDED Requirements

### Requirement: Dynamics Presets Are Independent of History
Loading a WvZz Dynamics factory preset SHALL first set every parameter to its default value and then apply the values the preset stores, so the result does not depend on the preset loaded before.

#### Scenario: Value from another preset does not stay
- **WHEN** a preset that turns `comp_phat` on is loaded, and then a preset that does not store `comp_phat`
- **THEN** `comp_phat` is off

#### Scenario: Same preset, same values
- **WHEN** any of the 20 presets is loaded directly, and again after another preset
- **THEN** all parameter values are identical in both cases

### Requirement: Dynamics Dice Sets Host Parameters
The WvZz Dynamics `Dice` trigger SHALL set the knob parameters of its six processing modules to random values inside musically safe bounds through the host parameter interface, so the user interface, the host, and the saved state follow. It SHALL NOT change bypass switches, mode selectors, the modulation section, or the Master section.

#### Scenario: Dice changes parameters within bounds
- **WHEN** Dice is triggered
- **THEN** the host parameter values change, `comp_thresh` is between `-30` and `-8 dB`, `comp_ratio` between `2` and `8`, and `clip_ceiling` between `-1.5` and `-0.1 dB`

#### Scenario: Dice leaves switches and Master alone
- **WHEN** Dice is triggered
- **THEN** `comp_bypass`, `comp_mode`, and `master_vol` keep their values
