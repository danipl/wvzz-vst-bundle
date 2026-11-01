# Plugin UI System Specification

## Purpose
Defines the visual design language, custom LookAndFeel vector styling, interactive controls, and real-time metering interface of the plugin editor.

## Requirements

### Requirement: Modern Vector Rotary Knobs
The UI system SHALL render custom vector rotary knobs featuring dynamic arc progress tracks (unipolar from minimum or bipolar from 12 o'clock center), center needle pointers, live parameter value readouts, and linear vertical drag tracking.

#### Scenario: User adjusts knob value
- **WHEN** user clicks and drags a rotary knob vertically or uses the mouse wheel
- **THEN** the knob arc fills smoothly (from minimum for unipolar parameters or from 12 o'clock center for bipolar parameters) and updates the underlying APVTS parameter value and readout in real time

### Requirement: Modular Section Grouping
The plugin editor SHALL organize the interface into a top header bar and the 3-column CamelPhat 3 layout with header-integrated module bypass controls:
- **Top Header Bar**: Plugin brand title, signal-flow chain summary (`BP → DIST → MM + BR → FLG → EQ → COMP → MASTER`), factory preset selector with previous/next navigation buttons, `Dice` randomizer button, and global `Master Bypass` toggle.
- **Left Column**: BP Filter (vertical dual sliders for Low/High Cut with visual passband shading, compact `LINK` pill toggle, Low/High Res knobs, BR Mix knob, header bypass button) stacked above Distortion (4 large rotary knobs for Mech, Tube, Bitcrusher, Xcita, header bypass button).
- **Center Column**: Interactive X/Y Pad at the top displaying active X and Y target names, paired with X/Y target selector dropdowns and bipolar depth knobs; distinct bordered LFO 1 and LFO 2 sub-panels directly below with live waveform visualizers, rate/sync controls that automatically switch between `Rate (Hz)` and `Sync Div`, bipolar depth knobs, and target dropdowns.
- **Right Column**: MM Filter section (12-item Filter Type dropdown, Cutoff, Res, Mix, Envelope Attack/Release/Mod, header bypass button) stacked above Flanger, Magic EQ (with glowing `PHAT` pill toggle), Compressor (with glowing `PHAT` pill toggle and real-time gain reduction dB meter), and Master Section (In Gain, Dry/Wet Mix, Out Volume, and Stereo VU/Peak Output Meter).

#### Scenario: Visual layout hierarchy
- **WHEN** the plugin window opens in any host DAW or a module bypass button is toggled
- **THEN** all modules appear in their designated Left, Center, or Right column positions with zero overlapping sibling controls, header-integrated bypass and `?` help triggers, and bypassed modules visibly dim their inner controls

### Requirement: Interactive Dual X/Y Vector Pads
The system SHALL provide a central interactive 2D X/Y Controller Pad (`xy_x`, `xy_y`) displaying the currently selected X and Y target names on the pad axes, paired with target selector dropdowns and bipolar depth controls (`xy_x_depth`, `xy_y_depth`) that modulate any target from the 21-item Modulatable Targets Enum.

#### Scenario: X/Y pad dragging
- **WHEN** the user selects targets for the X and Y axes and drags the puck inside the central X/Y Pad
- **THEN** the pad updates its X/Y axis labels to the selected target names and `xy_x` and `xy_y` update in real time to apply their configured depth offsets

### Requirement: Responsive 60 FPS Stereo VU Metering
The UI system SHALL provide smooth, hardware-accelerated stereo peak and RMS level meters driven by a UI timer reading atomic levels from the DSP master stage.

#### Scenario: Audio playback metering
- **WHEN** audio is processed through the plugin
- **THEN** the stereo meters animate fluidly at up to 60 FPS with peak hold and smooth ballistics without causing audio thread dropouts

### Requirement: Bidirectional Parameter Synchronization
All graphical controls SHALL maintain bidirectional synchronization with host DAW automation and preset recall using APVTS parameter attachments.

#### Scenario: Host automation movement
- **WHEN** a DAW automation lane modulates a parameter
- **THEN** the corresponding GUI knob, slider, or toggle updates its displayed position synchronously without fighting the host

### Requirement: Fixed-Aspect-Ratio Resizable Window
The plugin editor SHALL open at a default resolution of `1080 x 680 px` and support responsive window resizing from `75%` (`810 x 510 px`) to `200%` (`1920 x 1200 px`), scaling cleanly across both fixed-aspect-ratio drags and host-enforced window dimensions.

#### Scenario: User resizes plugin editor window
- **WHEN** the user drags the window resize corner or the host DAW resizes the editor window
- **THEN** the editor scales and centers the `1080 x 680` layout smoothly within the `810 x 510` to `1920 x 1200` bounds without control overlap or clipping

### Requirement: Live LFO Waveform Visualizers and Gain Reduction Meter
The UI system SHALL render live waveform and phase visualizers in the LFO 1 and LFO 2 sub-panels, a vertical passband shading indicator between the BP Filter dual vertical cutoff sliders, glowing `PHAT` pill toggles for Magic EQ and Compressor Phat modes, a Compressor real-time gain-reduction dB meter, and a header `Dice` randomizer button.

#### Scenario: Visual feedback during modulation and compression
- **WHEN** audio is compressed and LFO 1/2 are running
- **THEN** the LFO sub-panels animate the active oscillator waveform and phase while the Compressor gain-reduction meter displays real-time attenuation in dB
