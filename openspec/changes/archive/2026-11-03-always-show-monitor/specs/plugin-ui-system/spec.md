# Spec Delta

## MODIFIED Requirements

### Requirement: Modular Section Grouping
The plugin editor SHALL compose the interface from self-contained `IModuleView` components organized into a top header bar and the 3-column layout:
- **Top Header Bar**: Scalable vector `WvZzLogoComponent`, signal-flow chain indicator (`BP → DIST → MM + BR → FLG → EQ → COMP → MASTER`), factory preset selector with previous/next navigation buttons, `Dice` randomizer button, and global `Master Bypass` toggle.
- **Left Column**: BP Filter module view stacked above Distortion module view.
- **Center Column**: Signal Comparison Visualizer (Monitor) displayed as the default active upper view, with header tab switching (`[ X/Y PAD ]` / `[ MONITOR ]`) to the interactive X/Y Pad; distinct bordered LFO 1 and LFO 2 sub-panel views.
- **Right Column**: MM Filter module view stacked above Flanger, Magic EQ, Compressor (with real-time GR meter), and Master module view.

#### Scenario: Visual layout hierarchy
- **WHEN** the plugin window opens in any host DAW or a module bypass button is toggled
- **THEN** all module views position their controls within their card bounds in their designated column positions with zero overlapping sibling controls, the Center Column displays the real-time Signal Comparison Visualizer by default showing live wave changes, and bypassed modules visibly dim their controls
