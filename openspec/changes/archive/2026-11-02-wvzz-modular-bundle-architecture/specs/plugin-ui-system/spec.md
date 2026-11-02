# Spec Delta: plugin-ui-system

## ADDED Requirements

### Requirement: Unified Vector Brand Logo Display
The UI system SHALL provide a scalable vector logo component (`WvZzLogoComponent`) in the top header bar rendering the geometric WvZz brand waveform monogram and plugin subtitle using resolution-independent vector paths (`juce::Path`), scaling sharply across 75% to 200% window dimensions.

#### Scenario: Rendering brand logo at high DPI
- **WHEN** the plugin window renders or resizes between 75% and 200% UI scale
- **THEN** the WvZz logo badge and typography render with smooth anti-aliased vector paths without pixelation or bitmap blur

## MODIFIED Requirements

### Requirement: Modular Section Grouping
The plugin editor SHALL compose the interface from self-contained `IModuleView` components organized into a top header bar and the 3-column layout:
- **Top Header Bar**: Scalable vector `WvZzLogoComponent`, signal-flow chain indicator (`BP → DIST → MM + BR → FLG → EQ → COMP → MASTER`), factory preset selector with previous/next navigation buttons, `Dice` randomizer button, and global `Master Bypass` toggle.
- **Left Column**: BP Filter module view stacked above Distortion module view.
- **Center Column**: Dual-mode header tab switching between the interactive X/Y Pad and the Signal Comparison Visualizer; distinct bordered LFO 1 and LFO 2 sub-panel views.
- **Right Column**: MM Filter module view stacked above Flanger, Magic EQ, Compressor (with real-time GR meter), and Master module view.

#### Scenario: Visual layout hierarchy
- **WHEN** the plugin window opens in any host DAW or a module bypass button is toggled
- **THEN** all module views position their controls within their card bounds in their designated column positions with zero overlapping sibling controls, and bypassed modules visibly dim their controls
