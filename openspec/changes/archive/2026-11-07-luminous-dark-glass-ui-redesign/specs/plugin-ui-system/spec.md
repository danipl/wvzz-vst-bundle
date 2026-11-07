# Spec Delta: plugin-ui-system

## MODIFIED Requirements

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
The plugin editor SHALL compose the interface from self-contained `IModuleView` components housed within translucent dark glass cards (`#151a22` to `#10141a`) with top-edge hairline highlights and without hard horizontal header divider lines, organized into a top header bar and the 3-column layout:
- **Top Header Bar**: Scalable vector `WvZzLogoComponent`, dynamic signal-flow chain indicator (`BP → DIST → MM + BR → FLG → EQ → COMP → MASTER` or `BP → EQ → DIST → MM + BR → FLG → COMP → MASTER` when `meq_pre` is enabled), factory preset selector with previous/next navigation buttons, `Dice` randomizer button, and global `Master Bypass` toggle.
- **Left Column**: BP Filter module view stacked above Distortion module view (including `dist_mode` selector for `Classic / Clip / Fold`).
- **Center Column**: Signal Comparison Visualizer (Monitor) displayed as the default active upper view, with header tab switching (`[ X/Y PAD ]` / `[ MONITOR ]`) to the interactive X/Y Pad; distinct bordered LFO 1 and LFO 2 sub-panel views.
- **Right Column**: MM Filter module view stacked above Flanger, Magic EQ (with `PRE` routing and `PHAT` pill toggles), Compressor (with real-time GR meter), and Master module view (with `<120Hz MONO` and `CLIP` pill toggles).

#### Scenario: Visual layout hierarchy
- **WHEN** the plugin window opens in any host DAW or a module bypass button is toggled
- **THEN** all module views position their controls within their translucent card bounds in their designated column positions with zero overlapping sibling controls, the Center Column displays the real-time Signal Comparison Visualizer by default showing live wave changes, and bypassed modules visibly dim their controls and card surface

## ADDED Requirements

### Requirement: Curated Luminous Module Palette
The UI system SHALL assign a dedicated signature luminous accent color to each module card and its controls (Distortion: Coral Red, Magic EQ: Sky Cyan, Compressor: Honey Gold, MM Filter: Emerald Green, Flanger: Electric Violet, BP Filter: Warm Amber, Modulation/XY: Dual Cyan/Tangerine, Master: Arctic Ice White) rendered against a deep obsidian background (`#0d1015`), dynamically illuminating active tracks, toggle pills, and meter indicators.

#### Scenario: Visual differentiation across modules
- **WHEN** the plugin interface is displayed
- **THEN** each module's active rotary arcs, slider fills, and toggle badges glow with that module's designated signature accent color without color collisions across adjacent modules
