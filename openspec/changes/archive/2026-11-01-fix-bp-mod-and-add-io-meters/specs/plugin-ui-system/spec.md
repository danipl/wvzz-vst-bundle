# Spec Delta

## MODIFIED Requirements

### Requirement: Modular Section Grouping
The plugin editor SHALL organize the interface into a top header bar and the 3-column CamelPhat 3 layout with header-integrated module bypass controls:
- **Top Header Bar**: Plugin brand title, signal-flow chain summary (`BP → DIST → MM + BR → FLG → EQ → COMP → MASTER`), factory preset selector with previous/next navigation buttons, `Dice` randomizer button, and global `Master Bypass` toggle.
- **Left Column**: BP Filter (vertical dual sliders for Low/High Cut with visual passband shading, compact `LINK` pill toggle, Low/High Res knobs, BR Mix knob, header bypass button) stacked above Distortion (4 large rotary knobs for Mech, Tube, Bitcrusher, Xcita, header bypass button).
- **Center Column**: Interactive X/Y Pad at the top displaying active X and Y target names, paired with X/Y target selector dropdowns and bipolar depth knobs; distinct bordered LFO 1 and LFO 2 sub-panels directly below with live waveform visualizers, rate/sync controls that automatically switch between `Rate (Hz)` and `Sync Div`, bipolar depth knobs, and target dropdowns.
- **Right Column**: MM Filter section (12-item Filter Type dropdown, Cutoff, Res, Mix, Envelope Attack/Release/Mod, header bypass button) stacked above Flanger, Magic EQ (with glowing `PHAT` pill toggle), Compressor (with glowing `PHAT` pill toggle and real-time gain reduction dB meter), and Master Section (In Gain, Dry/Wet Mix, Out Volume, and dual Stereo VU/Peak Input and Output Meters).

#### Scenario: Visual layout hierarchy
- **WHEN** the plugin window opens in any host DAW or a module bypass button is toggled
- **THEN** all modules appear in their designated Left, Center, or Right column positions with zero overlapping sibling controls, header-integrated bypass and `?` help triggers, and bypassed modules visibly dim their inner controls

### Requirement: Responsive 60 FPS Stereo VU Metering
The UI system SHALL provide smooth, hardware-accelerated stereo peak and RMS level meters driven by a UI timer reading atomic levels from both the input stage and the master output stage.

#### Scenario: Audio playback metering
- **WHEN** audio is processed through the plugin
- **THEN** both the Input and Output stereo meters animate fluidly at up to 60 FPS with peak hold and smooth ballistics without causing audio thread dropouts

### Requirement: Live LFO Waveform Visualizers and Gain Reduction Meter
The UI system SHALL render live waveform and phase visualizers in the LFO 1 and LFO 2 sub-panels, a vertical passband shading indicator between the BP Filter dual vertical cutoff sliders that accurately follows the non-linear slider skew curves, glowing `PHAT` pill toggles for Magic EQ and Compressor Phat modes, a Compressor real-time gain-reduction dB meter, and a header `Dice` randomizer button.

#### Scenario: Visual feedback during modulation and compression
- **WHEN** audio is compressed and LFO 1/2 are running
- **THEN** the LFO sub-panels animate the active oscillator waveform and phase while the Compressor gain-reduction meter displays real-time attenuation in dB

#### Scenario: Passband visualizer slider tracking
- **WHEN** user moves either the Low Cut or High Cut vertical slider
- **THEN** the passband shaded rectangle updates smoothly matching the physical slider thumbs on the non-linear logarithmic scale without vertical inversion or offset error
