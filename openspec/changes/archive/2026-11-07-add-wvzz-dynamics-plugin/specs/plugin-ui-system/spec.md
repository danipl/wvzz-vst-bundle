## MODIFIED Requirements

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
