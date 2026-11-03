# Design

## Context

See `proposal.md` for background and motivation.
Currently, `ModulationView` in `Source/Modules/Modulation/ModulationView.cpp` (and `CamelPhatAudioProcessorEditor` in `Source/PluginEditor.cpp`) instantiates both the interactive X/Y Pad controls and the `SignalComparisonComponent` inside the Center Column card. The view constructor initializes tab state with `setCenterTab(false)`, which hides the `signalVisualizer` and shows the `xyPad` and target selector controls.

To make wave changes immediately and prominently visible to the user as requested, the initial view state should default to `showMonitor = true`, while maintaining full functionality for users to toggle back to `X/Y PAD`.

## Goals / Non-Goals

**Goals:**
- Set the default Center Column tab state to the Signal Monitor (`MONITOR`) on initialization in `ModulationView`.
- Ensure `card.setTitle("ANALYSIS & MONITOR")` and cyan accent coloration apply cleanly upon plugin launch.
- Ensure `signalVisualizer` is visible and properly laid out in `resized()` without requiring user interaction.
- Maintain seamless switching between `[ X/Y PAD ]` and `[ MONITOR ]` via the header tab buttons.
- Ensure consistency in both `ModulationView` (modular bundle architecture) and `PluginEditor` (standalone legacy view).

**Non-Goals:**
- Redesigning the 1080x680 canvas or changing the 3-column physical layout.
- Permanently removing the X/Y Pad controls or altering modulation routing parameters (`xy_x`, `xy_y`, `xy_x_target`, etc.).
- Modifying the DSP processing pipeline or audio thread behavior.

## Decisions

### Decision 1: Default `isMonitorActive` / `setCenterTab(true)` in `ModulationView`
- **Choice**: In `Source/Modules/Modulation/ModulationView.h`, default `isMonitorActive` to `true`, and in `ModulationView.cpp` call `setCenterTab(true)` at the end of the constructor instead of `setCenterTab(false)`. Also update `Source/PluginEditor.cpp` and `Source/PluginEditor.h` (`showingMonitor = true`, `setCenterTab(true)`) for symmetry.
- **Rationale**: Setting the monitor active by default achieves the user's objective ("show always the Monitor, as it helps a lot to see the waves changes") upon opening the plugin, while keeping the tab button for users who need the X/Y pad.
- **Alternatives considered**:
  - *Completely removing the X/Y Pad*: Would break modulation assignments and remove an important sound design feature.
  - *Adding a separate monitor window or third view*: Violates the fixed 1080x680 aspect ratio constraint and modular card design.

## Risks / Trade-offs

- **[Risk] Users wanting to drag the X/Y pad immediately need to click the `X/Y PAD` tab** → **Mitigation**: The tab button `[ X/Y PAD ]` remains visible and clearly labelled in the Center Column card header, styled with neon green accenting when hovered or clicked.
