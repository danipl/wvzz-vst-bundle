# Spec Delta

## ADDED Requirements

### Requirement: Fixed-Aspect-Ratio Resizable Window
The plugin editor SHALL open at a default resolution of `900 x 560 px` and support fixed-aspect-ratio window resizing from `75%` (`675 x 420 px`) to `200%` (`1800 x 1120 px`).

#### Scenario: User resizes plugin editor window
- **WHEN** the user drags the window resize corner in a host DAW
- **THEN** the editor scales smoothly while preserving the `900:560` aspect ratio within the `75%` to `200%` bounds

### Requirement: Live LFO Waveform Visualizers and Gain Reduction Meter
The UI system SHALL render live waveform visualizers in the LFO 1 and LFO 2 panels, a vertical passband shading indicator between the BP Filter dual vertical cutoff sliders, glowing `P` toggles for Magic EQ and Compressor Phat modes, a Compressor gain-reduction LED meter, and a Master `Dice` randomizer button.

#### Scenario: Visual feedback during modulation and compression
- **WHEN** audio is compressed and LFO 1/2 are running
- **THEN** the LFO panels animate the active oscillator waveform and phase while the Compressor LED meter displays real-time gain reduction in dB

## MODIFIED Requirements

### Requirement: Modular Section Grouping
The plugin editor SHALL organize the interface into the 3-column CamelPhat 3 layout:
- **Left Column**: BP Filter (vertical dual sliders for Low/High Cut with visual passband shading, Link toggle, Low/High Res knobs, BR Mix knob, bypass) stacked above Distortion (4 large rotary knobs for Mech, Tube, Bitcrusher, Xcita, bypass).
- **Center Column**: Single interactive X/Y Pad at the top with X/Y target selector dropdowns and depth controls; LFO 1 and LFO 2 panels directly below with live waveform visualizers, rate/sync controls, depth, and target dropdowns.
- **Right Column**: MM Filter section (12-item Filter Type dropdown, Cutoff, Res, Mix, Envelope Attack/Release/Mod, bypass) stacked above Flanger, Magic EQ (with glowing `P` toggle), Compressor (with glowing `P` toggle and gain reduction LED meter), and Master Section (Mix, Volume, Analog-style VU/Peak Output Meter, Dice button, bypass).

#### Scenario: Visual layout hierarchy
- **WHEN** the plugin window opens in any host DAW
- **THEN** all modules appear in their designated Left, Center, or Right column positions with individual bypass toggles and `?` help triggers

### Requirement: Interactive Dual X/Y Vector Pads
The system SHALL provide a central interactive 2D X/Y Controller Pad (`xy_x`, `xy_y`) with target selector dropdowns and bipolar depth controls (`xy_x_depth`, `xy_y_depth`) that modulate any target from the 21-item Modulatable Targets Enum.

#### Scenario: X/Y pad dragging
- **WHEN** the user selects targets for the X and Y axes and drags the puck inside the central X/Y Pad
- **THEN** `xy_x` and `xy_y` update in real time and apply their configured depth offsets to the selected target parameters
