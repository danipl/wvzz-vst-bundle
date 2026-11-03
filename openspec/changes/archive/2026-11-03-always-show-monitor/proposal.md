# Proposal

## Why

In `Plugins/WvZzDistortion`, monitoring the input and output wave changes in real time is essential for shaping sound, calibrating drive, and observing filter/distortion dynamics. Currently, the Signal Monitor is hidden behind a tabbed switch in the Center Column (`ModulationView`), defaulting to the X/Y Pad view so the user has to manually click `MONITOR` every time. Showing the Monitor by default or making it persistently visible/active ensures sound designers and producers immediately see wave transformations as soon as the plugin opens.

## What Changes

- Change default tab selection in `ModulationView` to show the Signal Monitor (`MONITOR`) by default on initialization.
- Ensure the Signal Monitor tab state and display persist properly so that users always see the waveform analysis view when opening or resetting the plugin.
- Keep the tab toggle controls (`[ X/Y PAD ]` / `[ MONITOR ]`) so users can still switch back to the interactive X/Y Pad when manual 2D puck modulation is needed, while keeping the Monitor as the primary/default visual state.

## Capabilities

### New Capabilities
<!-- No new system capabilities introduced -->

### Modified Capabilities
- `plugin-ui-system`: Update the Center Column modular section grouping requirements so that the Signal Comparison Visualizer (Monitor) is the primary/default active view in the Center Column, ensuring wave changes are immediately visible upon plugin launch.

## Impact

- `Source/Modules/Modulation/ModulationView.h` & `ModulationView.cpp`: Default `isMonitorActive` initialized to `true` / `setCenterTab(true)`.
- UI presentation in `Plugins/WvZzDistortion` (and any other editors consuming `ModulationView`): Central column displays real-time input/output oscilloscope analysis immediately on launch.
- No changes to real-time DSP, zero-allocation ring buffers, or APVTS parameters.
