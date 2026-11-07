## MODIFIED Requirements

### Requirement: Modular Section Grouping
The plugin editor SHALL compose the interface from self-contained `IModuleView` components organized into a top header bar and the 3-column layout:
- **Top Header Bar**: Scalable vector `WvZzLogoComponent`, dynamic signal-flow chain indicator (`BP → DIST → MM + BR → FLG → EQ → COMP → MASTER` or `BP → EQ → DIST → MM + BR → FLG → COMP → MASTER` when `meq_pre` is enabled), factory preset selector with previous/next navigation buttons, `Dice` randomizer button, and global `Master Bypass` toggle.
- **Left Column**: BP Filter module view stacked above Distortion module view (including `dist_mode` selector for `Classic / Clip / Fold`).
- **Center Column**: Signal Comparison Visualizer (Monitor) displayed as the default active upper view, with header tab switching (`[ X/Y PAD ]` / `[ MONITOR ]`) to the interactive X/Y Pad; distinct bordered LFO 1 and LFO 2 sub-panel views.
- **Right Column**: MM Filter module view stacked above Flanger, Magic EQ (with `PRE` routing and `PHAT` pill toggles), Compressor (with real-time GR meter), and Master module view (with `<120Hz MONO` and `CLIP` pill toggles).

#### Scenario: Visual layout hierarchy
- **WHEN** the plugin window opens in any host DAW or a module bypass button is toggled
- **THEN** all module views position their controls within their card bounds in their designated column positions with zero overlapping sibling controls, the Center Column displays the real-time Signal Comparison Visualizer by default showing live wave changes, and bypassed modules visibly dim their controls
