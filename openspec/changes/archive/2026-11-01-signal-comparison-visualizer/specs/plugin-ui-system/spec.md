# Spec Delta

## MODIFIED Requirements

### Requirement: Modular Section Grouping
The plugin editor SHALL organize the interface into a top header bar and the 3-column CamelPhat 3 layout with header-integrated module bypass controls:
- **Top Header Bar**: Plugin brand title, signal-flow chain summary (`BP → DIST → MM + BR → FLG → EQ → COMP → MASTER`), factory preset selector with previous/next navigation buttons, `Dice` randomizer button, and global `Master Bypass` toggle.
- **Left Column**: BP Filter (vertical dual sliders for Low/High Cut with visual passband shading, compact `LINK` pill toggle, Low/High Res knobs, BR Mix knob, header bypass button) stacked above Distortion (4 large rotary knobs for Mech, Tube, Bitcrusher, Xcita, header bypass button).
- **Center Column**: Dual-mode header tab switching between the interactive X/Y Pad and the Signal Comparison Visualizer; distinct bordered LFO 1 and LFO 2 sub-panels directly below with live waveform visualizers, rate/sync controls that automatically switch between `Rate (Hz)` and `Sync Div`, bipolar depth knobs, and target dropdowns.
- **Right Column**: MM Filter section (12-item Filter Type dropdown, Cutoff, Res, Mix, Envelope Attack/Release/Mod, header bypass button) stacked above Flanger, Magic EQ (with glowing `PHAT` pill toggle), Compressor (with glowing `PHAT` pill toggle and real-time gain reduction dB meter), and Master Section (In Gain, Dry/Wet Mix, Out Volume, and dual Stereo VU/Peak Input and Output Meters).

#### Scenario: Visual layout hierarchy
- **WHEN** the plugin window opens in any host DAW or a module bypass button is toggled
- **THEN** all modules appear in their designated Left, Center, or Right column positions with zero overlapping sibling controls, header-integrated bypass and `?` help triggers, and bypassed modules visibly dim their inner controls
