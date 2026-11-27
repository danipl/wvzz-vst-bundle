# Plugin UI System Specification

## Purpose
Defines the visual design language, custom LookAndFeel vector styling, interactive controls, and real-time metering interface of the plugin editor.

## Requirements

### Requirement: Modern Vector Rotary Knobs
The UI system SHALL render custom vector rotary knobs featuring flat dark matte caps, perimeter tick pointers, dynamic two-pass glowing arc progress tracks (unipolar from minimum or bipolar from 12 o'clock center), responsive hover brightness boosts, live parameter value readouts, and linear vertical drag tracking.

#### Scenario: User adjusts knob value
- **WHEN** user clicks and drags a rotary knob vertically or uses the mouse wheel
- **THEN** the knob arc fills smoothly (from minimum for unipolar parameters or from 12 o'clock center for bipolar parameters) with a multi-pass vector glow and updates the underlying APVTS parameter value and readout in real time

#### Scenario: User hovers over rotary knob
- **WHEN** user hovers the cursor over any rotary knob or begins dragging
- **THEN** the knob arc glow amplifies in brightness and the numeric readout illuminates in high-contrast text

#### Scenario: User double-clicks knob for direct numeric entry
- **WHEN** user double-clicks any rotary knob
- **THEN** an inline text editor opens centered over the knob allowing the user to type an exact numeric parameter value

### Requirement: Modular Section Grouping
The plugin editor SHALL compose the interface from self-contained `IModuleView` components housed within translucent dark glass cards (`#151a22` to `#10141a`) with top-edge hairline highlights and without hard horizontal header divider lines, organized into a top header bar and a 3-column layout tailored to each plugin:
- **Top Header Bar**: Scalable vector `WvZzLogoComponent` displaying the plugin subtitle (`DISTORTION` or `DYNAMICS`), dynamic signal-flow chain indicator (`BP → DIST → MM + BR → FLG → EQ → COMP → MASTER` for Distortion; `DET → TRANS → OTT → COMP → PUMP → CLIP → MASTER` for Dynamics), factory preset selector with previous/next navigation buttons, `Dice` randomizer button, and global `Master Bypass` toggle.
- **WvZz Distortion Columns**:
  - **Left Column**: BP Filter module view stacked above Distortion module view.
  - **Center Column**: Signal Comparison Visualizer (`MONITOR`) displayed as the default active upper view with header tab switching (`[ X/Y PAD ]` / `[ MONITOR ]`), above LFO 1 and LFO 2 sub-panel views.
  - **Right Column**: MM Filter stacked above Flanger, Magic EQ, Compressor, and Master module view.
- **WvZz Dynamics Columns**:
  - **Left Column**: Sub & Detector (`DynDetectorModule`) stacked above Transient & Gate (`TransientShaperModule`) and 3-Band Hyper-OTT (`MultibandOTTModule`).
  - **Center Column**: Signal Comparison Visualizer (`MONITOR` with `SCOPE`, `SPECTRUM`, `DELTA`, and `GR` trace + live Crest Factor dB badge) displayed by default with header tab switching (`[ X/Y PAD ]` / `[ MONITOR ]`), above LFO 1 and LFO 2 sub-panel views.
  - **Right Column**: Multi-Mode Bus Compressor (`CompressorModule`) stacked above Rhythmic Pumper (`RhythmicPumperModule`), Maximizer Clipper (`MaximizerClipperModule`), and Master (`MasterModule`) view.

#### Scenario: Visual layout hierarchy
- **WHEN** either plugin window (`WvZz Distortion` or `WvZz Dynamics`) opens in any host DAW or a module bypass button is toggled
- **THEN** all module views position their controls within their translucent card bounds in their designated column positions with zero overlapping sibling controls, the Center Column displays the real-time Signal Comparison Visualizer by default, and bypassed modules visibly dim their controls and card surface

### Requirement: Interactive Dual X/Y Vector Pads
The system SHALL provide a central interactive 2D X/Y Controller Pad (`xy_x`, `xy_y`) displaying the currently selected X and Y target names on the pad axes, paired with target selector dropdowns and bipolar depth controls (`xy_x_depth`, `xy_y_depth`) that modulate any target from the 21-item Modulatable Targets Enum.

#### Scenario: X/Y pad dragging
- **WHEN** the user selects targets for the X and Y axes and drags the puck inside the central X/Y Pad
- **THEN** the pad updates its X/Y axis labels to the selected target names and `xy_x` and `xy_y` update in real time to apply their configured depth offsets

### Requirement: Responsive 60 FPS Stereo VU Metering
The UI system SHALL provide smooth, hardware-accelerated stereo peak and RMS level meters driven by a UI timer reading atomic levels from both the input stage and the master output stage.

#### Scenario: Audio playback metering
- **WHEN** audio is processed through the plugin
- **THEN** both the Input and Output stereo meters animate fluidly at up to 60 FPS with peak hold and smooth ballistics without causing audio thread dropouts

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
The UI system SHALL render live waveform and phase visualizers in the LFO 1 and LFO 2 sub-panels, a vertical passband shading indicator between the BP Filter dual vertical cutoff sliders, glowing `PHAT` pill toggles for Magic EQ and Compressor Phat modes, a header `Dice` randomizer button, and a Compressor visualizer featuring a 5-knob layout (`Threshold`, `Ratio`, `Attack`, `Release`, `Mix`), calibrated decibel scale tick marks (`0`, `-3`, `-6`, `-12`, `-18`, `-24 dB`), analog meter ballistics with a floating peak-hold line, a dynamic numeric decibel readout, and visual saturation glow on Phat clipping.

#### Scenario: Visual feedback during modulation and compression
- **WHEN** audio is compressed and LFO 1/2 are running
- **THEN** the LFO sub-panels animate the active oscillator waveform and phase while the Compressor meter displays real-time attenuation in dB with calibrated scale marks, smooth ballistic decay, and a floating peak-hold indicator

#### Scenario: Passband visualizer slider tracking
- **WHEN** user moves either the Low Cut or High Cut vertical slider
- **THEN** the passband shaded rectangle updates smoothly matching the physical slider thumbs on the non-linear logarithmic scale without vertical inversion or offset error

#### Scenario: Calibrated compressor metering and numeric readout
- **WHEN** audio compression occurs in the Compressor module
- **THEN** the GR meter animates with rapid descent and smooth fallback across the calibrated `-24 dB` scale, and the numeric readout updates with the active gain reduction value

### Requirement: Unified Vector Brand Logo Display
The UI system SHALL provide a scalable vector logo component (`WvZzLogoComponent`) in the top header bar rendering the geometric WvZz brand waveform monogram and plugin subtitle using resolution-independent vector paths (`juce::Path`), scaling sharply across 75% to 200% window dimensions.

#### Scenario: Rendering brand logo at high DPI
- **WHEN** the plugin window renders or resizes between 75% and 200% UI scale
- **THEN** the WvZz logo badge and typography render with smooth anti-aliased vector paths without pixelation or bitmap blur

### Requirement: Curated Luminous Module Palette
The UI system SHALL assign a dedicated signature luminous accent color to each module card and its controls (Distortion: Coral Red, Magic EQ: Sky Cyan, Compressor: Honey Gold, MM Filter: Emerald Green, Flanger: Electric Violet, BP Filter: Warm Amber, Modulation/XY: Dual Cyan/Tangerine, Master: Arctic Ice White) rendered against a deep obsidian background (`#0d1015`), dynamically illuminating active tracks, toggle pills, and meter indicators.

#### Scenario: Visual differentiation across modules
- **WHEN** the plugin interface is displayed
- **THEN** each module's active rotary arcs, slider fills, and toggle badges glow with that module's designated signature accent color without color collisions across adjacent modules

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

### Requirement: Delay Tap Display
WvZz Delay SHALL show a delay-tap display as its custom center tab: a time axis with one marker per repeat for the left and the right channel, where marker height follows the feedback decay. The display SHALL read the delay state through lock-free atomics published by the audio thread.

#### Scenario: Taps follow time and feedback
- **WHEN** the delay time or feedback changes
- **THEN** the markers move to the new repeat times and their heights follow the new feedback

#### Scenario: Ping-pong shows alternating sides
- **WHEN** `dly_routing` is `Ping-Pong`
- **THEN** the markers alternate between the left lane and the right lane

### Requirement: WvZz Delay Editor Layout
The WvZz Delay editor SHALL use the shared `1080 x 680` shell with the header toolbar and one card per module: Delay in the left column, the modulation and monitor card in the center, and Grain and Master in the right column.

#### Scenario: Editor opens at the base size
- **WHEN** the editor is created
- **THEN** its size is `1080 x 680` and it shows the Delay, Grain, center, and Master cards and exactly one delay-tap display

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
