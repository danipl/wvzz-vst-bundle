# Tasks

## 1. Modulation View Default State Update

- [x] 1.1 In `Source/Modules/Modulation/ModulationView.h`, update default member state `bool isMonitorActive { true };` to reflect the Monitor being the default active tab.
- [x] 1.2 In `Source/Modules/Modulation/ModulationView.cpp`, change the initial tab call from `setCenterTab(false)` to `setCenterTab(true)` in the constructor so the Signal Comparison Visualizer is active, visible, and styled with cyan accenting upon view creation.
- [x] 1.3 In `Source/PluginEditor.h` and `Source/PluginEditor.cpp`, update `showingMonitor` default to `true` and call `setCenterTab(true)` on initialization for backward compatibility and visual parity with the legacy editor.

## 2. Verification and Visual Checks

- [x] 2.1 Build the WvZz Distortion standalone target or plugin tests using CMake (`cmake --build build --target WvZzDistortion_Standalone` or `ctest`) and verify compilation succeeds with zero warnings/errors.
- [x] 2.2 Verify that upon launching the plugin or instantiating the editor, the Center Column displays the real-time Signal Comparison Visualizer (Monitor) by default, showing live input/output wave traces, and clicking `[ X/Y PAD ]` toggles to the X/Y pad smoothly.
