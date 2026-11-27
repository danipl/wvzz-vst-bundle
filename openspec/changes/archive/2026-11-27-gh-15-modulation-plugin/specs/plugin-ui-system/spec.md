# Spec Delta

## ADDED Requirements

### Requirement: Plugin-Specific Custom Display Tab
The center column SHALL accept one optional plugin-specific display, shown as a third tab next to the `X/Y PAD` and `MONITOR` tabs. Plugins that provide no custom display SHALL keep the two-tab layout unchanged.

#### Scenario: Plugin with a custom display
- **WHEN** a plugin registers a custom display with a tab title
- **THEN** the center column header shows three tabs and selecting the third tab shows the custom display in the area used by the X/Y pad and the monitor

#### Scenario: Plugin without a custom display
- **WHEN** WvZz Distortion or WvZz Dynamics opens its editor
- **THEN** the center column shows only the `X/Y PAD` and `MONITOR` tabs at their previous positions

### Requirement: LFO Phase Scope
WvZz Modulation SHALL show an LFO phase scope as its custom display: one lane per effect (Chorus, Phaser, Flanger, Tremolo) with the LFO waveform over one cycle and a marker for the current left and right LFO position. The scope SHALL read the LFO state through lock-free atomics published by the audio thread and SHALL dim the lane of a bypassed effect.

#### Scenario: Marker follows the LFO
- **WHEN** an effect is active and audio is running
- **THEN** the lane marker moves through the cycle at the effect's rate, and the right-channel marker is offset by the effect's stereo phase

#### Scenario: Bypassed effect
- **WHEN** an effect is bypassed
- **THEN** its lane is drawn dimmed

### Requirement: WvZz Modulation Editor Layout
The WvZz Modulation editor SHALL use the shared `1080 x 680` shell with the header toolbar and one card per module: Chorus and Phaser in the left column, the modulation and monitor card in the center, and Flanger, Tremolo, and Master in the right column.

#### Scenario: Editor opens at the base size
- **WHEN** the editor is created
- **THEN** its size is `1080 x 680` and it shows cards for Chorus, Phaser, Flanger (with feedback, depth, center, and through-zero controls), Tremolo, the center column, and Master
