# Proposal

## Why

The current CamelPhat 3 plugin editor has several layout, rendering, and installation issues that make the interface cramped, partially unpainted, and prone to loading stale plugin bundles on macOS:
1. `CamelPhatAudioProcessorEditor::paint()` is empty and `contentContainer` is a bare `juce::Component`, leaving the main window background and top branding bar unpainted.
2. The `900 x 560` canvas packs four stacked rows of modules in the right column (`MM Filter`, `Flanger` + `Magic EQ`, `Compressor`, `Master`) and places full-width `BYPASS` switch rows inside card bodies, starving rotary knobs down to `25–28 px` height and clipping toggle labels in narrow cards.
3. Spec-required visualizers—the `BP Filter` vertical passband shader, `LFO 1 & 2` waveform/phase visualizers, `Compressor` gain-reduction LED meter, bipolar 12-o'clock knob arcs, and dynamic `X/Y Pad` axis target readouts—are either missing or unstyled.
4. `HelpOverlayComponent` computes screen-to-local coordinates against the unscaled parent editor rather than the scaled `contentContainer`, causing help popups to misalign when the window is resized.
5. Stale plugin bundles in `~/Library/Audio/Plug-Ins/` shadow `/Library/Audio/Plug-Ins/` on macOS, causing hosts to load outdated builds.

## What Changes

- **Expanded Responsive 3-Column Layout (`1080 x 680` base canvas)**: Upgrade the base canvas to `1080 x 680 px` (resizable from `810 x 510 px` [75%] up to `1920 x 1200 px`) with a responsive `ContentContainer` that paints the dark base background, top branding/signal-flow header bar (`48 px`), and scales/lays out all 3 columns (`28% / 34% / 38%`) cleanly at any host window size.
- **Top Header & Preset Toolbar**: Render the `CAMELPHAT` neon header, signal-flow chain indicator (`BP → DIST → MM + BR → FLG → EQ → COMP → MASTER`), preset selector with `<` / `>` step buttons, `DICE` smart randomizer button, and global `BYPASS` toggle in the top bar.
- **Header-Integrated Card Bypass & Bypassed Dimming**: Move per-module `BYPASS` switches out of the card interior into each `ModuleCardComponent` header bar as a compact illuminated power/active LED pill button next to the `?` help trigger, and dim bypassed module controls (`alpha = 0.45f`) for instant visual feedback.
- **Bipolar Knobs, Value Readouts & Compact Pill Toggles**: Update `CamelPhatLookAndFeel::drawRotarySlider` to draw bipolar arcs originating from 12 o'clock (`0.5` normalized) for bipolar parameters (`xy_x_depth`, `xy_y_depth`, `lfo1_depth`, `lfo2_depth`, `mm_env_depth`, `meq_amount`), add custom `drawLinearSlider` and `drawButtonBackground`/`drawToggleButton` for compact illuminated pill buttons (`PHAT`, `LINK`, `SYNC`), and ensure every knob has generous vertical space (`>= 56 px`).
- **Interactive Visualizers & Meters**:
  - **BP Filter**: Custom vertical `Low Cut` and `High Cut` sliders with a live shaded passband region between them.
  - **Modulation & X/Y Pad**: Dynamic X and Y target name labels on the `XYPadComponent` plus distinct bordered `LFO 1` and `LFO 2` sub-panels with live animated waveform/phase visualizers and automatic `Rate (Hz)` vs. `Sync Div` control switching when `SYNC` is toggled.
  - **Compressor Gain Reduction Meter**: Atomic gain-reduction telemetry in `CompressorModule` feeding a real-time vertical dB Gain Reduction meter inside the `COMPRESSOR` card.
- **Scaled Help Overlay Positioning**: Fix `HelpOverlayComponent::displayTopic` to transform coordinates into `contentContainer` local space so `?` help cards align accurately at any window scale factor.
- **Build & Install Sync + Automated UI Layout Tests**: Update `Makefile` so `make install` synchronizes both `~/Library/Audio/Plug-Ins/` and `/Library/Audio/Plug-Ins/` (when writable) and add automated editor layout and non-overlap tests to `Tests/DSPTests.cpp`.

## Capabilities

### New Capabilities

### Modified Capabilities
- `plugin-ui-system`: Updates the base editor resolution (`1080 x 680 px`, resizable `75%`–`200%` and responsive to host bounds), top header toolbar, card-header bypass buttons with bypassed state dimming, bipolar rotary knob arcs, pill toggles, BP passband visualizer, LFO 1/2 sub-panels with waveform visualizers and Rate/Sync switching, dynamic X/Y pad axis labels, and Compressor gain-reduction metering.
- `component-help-system`: Updates contextual help overlay positioning to map screen bounds into the scaled canvas coordinate space so overlays anchor accurately at any window zoom level.

## Impact

- **Code**: `Source/PluginProcessor.h`, `Source/DSP/CompressorModule.h`, `Source/DSP/ModulationEngine.h`, `Source/PluginEditor.h`, `Source/PluginEditor.cpp`, `Source/UI/CamelPhatLookAndFeel.h`, `Source/UI/CamelPhatLookAndFeel.cpp`, `Source/UI/ModuleCardComponent.h`, `Source/UI/XYPadComponent.h`, `Source/UI/XYPadComponent.cpp`, `Source/UI/HelpOverlayComponent.h`, `Tests/DSPTests.cpp`, and `Makefile`.
- **Compatibility**: Preserves 100% compatibility with all existing APVTS parameter IDs, automation lanes, and factory presets.
