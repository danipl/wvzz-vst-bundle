# Tasks

## 1. DSP Telemetry for UI Visualizers

- [x] 1.1 Expose thread-safe atomic Gain Reduction telemetry (`getGainReductionDb()`) in `Source/DSP/CompressorModule.h` (clearing attenuation to `0.0f` when bypassed or silent) and expose `getCompressorModule()` on `CamelPhatAudioProcessor` in `Source/PluginProcessor.h`; verify with a unit test in `Tests/DSPTests.cpp` that `getGainReductionDb()` reports negative dB during compression and `0.0f` when bypassed.
- [x] 1.2 Expose normalized LFO phase getters (`getLfo1Phase()`, `getLfo2Phase()`) alongside output snapshots in `Source/DSP/ModulationEngine.h`; verify with `make test` that LFO phase advances in `[0.0, 1.0)` without audio-thread allocations.

## 2. Custom LookAndFeel, Card Chrome & Visualizers

- [x] 2.1 Upgrade `Source/UI/CamelPhatLookAndFeel.h` and `Source/UI/CamelPhatLookAndFeel.cpp` to render bipolar 12-o'clock rotary knob arcs (`isBipolar` property), live parameter value readouts, custom vertical sliders (`drawLinearSlider`), and compact illuminated pill buttons (`drawToggleButton` / `drawButtonBackground` for `ON/BYP` header pills, `PHAT`, `LINK`, `SYNC`, and `DICE`); verify compilation with `cmake --build build`.
- [x] 2.2 Enhance `Source/UI/ModuleCardComponent.h` to support bypassed state dimming, header-integrated bypass toggle bounds, and optional custom background overlays; verify header layout metrics in `ModuleCardComponent`.
- [x] 2.3 Update `Source/UI/XYPadComponent.h` and `Source/UI/XYPadComponent.cpp` to display dynamic X and Y target names (`setAxisLabels`) and bipolar crosshair/glow styling; verify `XYPadComponent` compiles cleanly.
- [x] 2.4 Fix `Source/UI/HelpOverlayComponent.h` to compute local popup placement using the scaled canvas coordinate space (`getLocalArea(nullptr, targetScreenBounds)`) clamped to the `1080 x 680` canvas; verify popup bounds stay inside `[0, 1080] x [0, 680]` across window scale factors.

## 3. Responsive `1080 x 680` 3-Column Editor Layout & Sub-Panels

- [x] 3.1 Redesign `Source/PluginEditor.h` and `Source/PluginEditor.cpp` around a `1080 x 680` painted `ContentContainer` (resizable `810 x 510` to `1920 x 1200` with aspect-fit centered scaling), top branding/signal-flow/preset/Dice/Master-Bypass header bar (`48 px`), and 3-column layout (`Left`: BP Filter with passband shader + Distortion; `Center`: X/Y Pad + bordered LFO 1 & LFO 2 sub-panels with live waveform visualizers and Rate/SyncDiv switching; `Right`: MM Filter + Flanger & Magic EQ + Compressor with GR dB meter + Master with Stereo VU meter); verify `cmake --build build` succeeds with zero warnings.
- [x] 3.2 Add an automated GUI layout and bounds verification test in `Tests/DSPTests.cpp` that instantiates `CamelPhatAudioProcessorEditor`, tests resizing at `810 x 510`, `1080 x 680`, and `1920 x 1200`, and asserts every visible control has valid bounds and zero sibling controls overlap; run `make test` and confirm all tests pass.

## 4. Build, Plugin Installation Sync & Validation

- [x] 4.1 Update `Makefile` so building and installing synchronizes `~/Library/Audio/Plug-Ins/VST3/CamelPhat.vst3`, `~/Library/Audio/Plug-Ins/Components/CamelPhat.component`, and `/Library/Audio/Plug-Ins/VST3/CamelPhat.vst3` (when writable), and run `openspec validate "responsive-ui-redesign" --strict` to confirm all spec deltas and artifacts pass validation.

## Workflow follow-up

- Run `/opsx-apply` to execute all tasks, build/install the updated VST3/AU/Standalone targets, and run the automated test suite.
- Archive the change with `/opsx-archive` after verification is complete.
