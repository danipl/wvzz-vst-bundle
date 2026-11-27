# Spec Delta

## ADDED Requirements

### Requirement: Reverb Decay-Curve Display
WvZz Reverb SHALL show a decay-curve display as its custom center tab: level in dB against time, with the pre-delay gap, the early-reflections region, and one decay line each for low, mid, and high frequencies. The display SHALL read the reverb state through lock-free atomics published by the audio thread.

#### Scenario: Curve follows the decay setting
- **WHEN** `rev_decay` changes
- **THEN** the mid line reaches `-60 dB` at the pre-delay time plus the new decay time

#### Scenario: Damping bends the low and high lines
- **WHEN** `rev_hidamp` is lowered
- **THEN** the high line falls faster than the mid line

### Requirement: WvZz Reverb Editor Layout
The WvZz Reverb editor SHALL use the shared `1080 x 680` shell with the header toolbar and one card per module: Reverb in the left column, the modulation and monitor card in the center, and Early Reflections and Master in the right column.

#### Scenario: Editor opens at the base size
- **WHEN** the editor is created
- **THEN** its size is `1080 x 680` and it shows the Reverb, Early Reflections, center, and Master cards and exactly one decay-curve display
