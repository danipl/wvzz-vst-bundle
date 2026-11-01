# Design

## Context

See `proposal.md` for why the redesign is needed. The current editor (`Source/PluginEditor.cpp`) uses an unpainted `900 x 560` `contentContainer`, places module `BYPASS` switches inside card content areas, lacks custom styling for linear sliders, bipolar knobs, LFO visualizers, and Compressor gain-reduction metering, and miscalculates `HelpOverlayComponent` screen-to-local coordinates when scaled. Additionally, `Makefile` installs into `~/Library/Audio/Plug-Ins/` only on `make install`, while `/Library/Audio/Plug-Ins/VST3` can drift out of sync.

## Goals / Non-Goals

**Goals:**
- Provide a spacious, legible `1080 x 680` 3-column canvas where every rotary knob has at least `56 px` of vertical height and zero sibling controls overlap.
- Integrate module `BYPASS` toggles directly into `ModuleCardComponent` headers as illuminated power pills and dim bypassed module controls (`alpha = 0.45f`).
- Implement missing visualizers required by the spec: BP Filter passband shading + custom vertical sliders, LFO 1 & 2 waveform/phase visualizers + `Rate`/`Sync Div` auto-visibility, X/Y Pad dynamic target labels, and Compressor Gain Reduction meter.
- Ensure responsive, aspect-preserving scaling for any host window size (`810 x 510` to `1920 x 1200`) with accurate coordinate mapping for floating `?` help overlays.
- Verify layout bounds programmatically in `Tests/DSPTests.cpp` and keep macOS plugin install paths synchronized.

**Non-Goals:**
- Changing audio DSP transfer functions or APVTS parameter IDs/ranges.
- Adding external multi-window popups or OpenGL shader dependencies (all rendering uses hardware-accelerated 2D `juce::Graphics` paths).

## Decisions

### 1. `1080 x 680` Logical Canvas with Aspect-Fit Responsive Scaling
- **Choice**: Upgrade the logical canvas size from `900 x 560` to `1080 x 680` (with resize limits `810 x 510` to `1920 x 1200`) and compute `scale = std::min(getWidth() / 1080.0f, getHeight() / 680.0f)` with centered letterboxing inside `CamelPhatAudioProcessorEditor::resized()`.
- **Rationale**: `1080 x 680` provides `+120 px` of vertical height and `+180 px` of horizontal width over `900 x 560`, eliminating the `25 px` knob starvation in the right column while still fitting comfortably on 13-inch laptop screens (`1440 x 900` logical). Using `std::min(scaleX, scaleY)` guarantees that even if a DAW host forces an arbitrary non-16:10 window size, no controls clip or stretch.
- **Alternatives considered**: Pure `FlexBox` reflow without canvas scaling — rejected because JUCE font sizes, combo box popups, and knob stroke thicknesses do not scale automatically under raw `FlexBox` bounds without an `AffineTransform`.

### 2. Header-Integrated Bypass & Dimming in `ModuleCardComponent`
- **Choice**: Place each module's bypass `ToggleButton` in the top-right of its `ModuleCardComponent` header bar (to the left of the `?` `HelpButtonComponent`), styled via a `"isHeaderBypass"` component property in `CamelPhatLookAndFeel::drawToggleButton` as a compact `ON` / `BYP` illuminated LED badge. When `bypass == true`, the editor updates the card's bypassed state and sets `alpha = 0.45f` on the module's child controls.
- **Rationale**: Reclaims `20–24 px` of vertical height inside all 7 module cards and makes active vs. bypassed signal-chain stages immediately scannable.
- **Alternatives considered**: Keeping bypass switches inside card bodies — rejected because narrow cards (`Flanger`, `Magic EQ`) clip text and lose vertical space needed for rotary dials.

### 3. Bipolar Rotary Arcs & Compact Pill Toggles in `CamelPhatLookAndFeel`
- **Choice**:
  - In `initRotary()`, set `ctrl.slider.getProperties().set("isBipolar", isBipolar)` and `accentColor` matching the parent card. In `CamelPhatLookAndFeel::drawRotarySlider`, draw bipolar value arcs starting from `rotaryStartAngle + 0.5f * (rotaryEndAngle - rotaryStartAngle)` (12 o'clock) with a center tick mark, and render the current formatted parameter value string in a subtle pill or sub-label at the bottom of the knob dial.
  - Add `CamelPhatLookAndFeel::drawLinearSlider` for the `BP Filter` vertical sliders and `drawToggleButton` support for compact illuminated pill buttons (`LINK`, `PHAT`, `SYNC`).
- **Rationale**: Visualizes positive vs. negative modulation depth (`-100%` to `+100%`) and EQ cut/boost immediately without requiring hover popups.

### 4. Lightweight Custom Visualizers (`BPPassband`, `LFOWaveform`, `GainReductionMeter`)
- **Choice**:
  - **BP Passband**: Paint a translucent cyan passband rectangle and frequency response curve between the `Low Cut` and `High Cut` slider thumb Y positions inside `BPFilterCard`.
  - **LFO Sub-Panels**: Expose `getLfo1Phase()` / `getLfo2Phase()` (atomic `float` in `[0, 1)`) alongside `getLfo1Snapshot()` / `getLfo2Snapshot()` in `ModulationEngine`, and render a compact 60 FPS oscilloscope/waveform preview box inside each LFO sub-panel. Toggle visibility between `lfoRateKnob` and `lfoSyncDivBox` based on `lfoSyncToggle` so only the active rate control occupies the rate slot (or highlight the active one clearly).
  - **Compressor Gain Reduction Meter**: Store `gainReductionDb` and `envelope` atomically in `CompressorModule` (resetting `gainReductionDb` to `0.0f` when bypassed or silent) and render a vertical amber/red downward dB attenuation meter in `compCard`.
- **Rationale**: Zero heap allocations on the audio thread (`std::atomic<float>` with `memory_order_relaxed`) and crisp 30–60 FPS vector rendering on the message thread.

### 5. Scaled Coordinate Mapping for `HelpOverlayComponent`
- **Choice**: Pass `&contentContainer` as the coordinate reference component to `HelpOverlayComponent::displayTopic` (using `contentContainer.getLocalArea(nullptr, targetScreenBounds)` instead of `parentEditor->getLocalPoint`), and constrain the popup within `1080 x 680`.
- **Rationale**: Since `helpOverlay` is a child of `contentContainer`, using `contentContainer.getLocalArea` automatically inverts `contentContainer`'s `AffineTransform::scale`, positioning the help card accurately at any zoom factor (`75%`–`200%`).

## Risks / Trade-offs

- **[Risk] Stale macOS user plugin bundles (`~/Library/Audio/Plug-Ins/`) shadowing system bundles (`/Library/Audio/Plug-Ins/`)** → **Mitigation**: Update `Makefile` so `make build` / `make install` always updates and re-signs `~/Library/Audio/Plug-Ins/VST3/CamelPhat.vst3` and `~/Library/Audio/Plug-Ins/Components/CamelPhat.component` (and `/Library/Audio/Plug-Ins/VST3/CamelPhat.vst3` if writable).
- **[Risk] Control overlap regression when modifying layout bounds** → **Mitigation**: Add a dedicated unit test in `Tests/DSPTests.cpp` that instantiates `CamelPhatAudioProcessorEditor`, resizes it across `810x510`, `1080x680`, and `1920x1200`, and asserts that all visible controls have positive dimensions and non-overlapping sibling bounds.
