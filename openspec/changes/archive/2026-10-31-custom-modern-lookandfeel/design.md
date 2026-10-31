# Design

## Context
See `proposal.md` for background and user requirements. The core audio engine and APVTS parameter architecture are fully implemented and verified with zero heap allocations. This design specifies the visual architecture, custom LookAndFeel, modular UI hierarchy, dual X/Y pads, real-time metering, and contextual "?" help system for `CamelPhatAudioProcessorEditor`.

## Goals / Non-Goals

**Goals:**
- Provide a modern, intuitive, vector-based custom LookAndFeel (`CamelPhatLookAndFeel`) with dark/neon aesthetic, ergonomic rotary knobs, numeric readouts, and styled toggle buttons.
- Implement a contextual in-plugin help system with interactive "?" buttons on every module card, rendering plain-language explanations of DSP behavior and parameters.
- Provide interactive dual X/Y vector control pads for paired parameter modulation (Filter Cutoff vs Resonance; Tube vs Mech drive).
- Provide smooth 60 FPS animated stereo peak/RMS VU meters reading lock-free atomic levels from the DSP engine's `IMeterSource`.
- Maintain clean SOLID principles with decoupled interface contracts (`IHelpContentProvider`, `IHelpOverlayHost`).

**Non-Goals:**
- Heavy raster/bitmap asset packs (all graphics are resolution-independent pure vector drawing).
- Complex multi-tabbed preference pages (all 6 DSP modules, X/Y pads, and master section fit cohesively in a single-view panoramic control surface).

## Decisions

### 1. SOLID Interface Architecture for Contextual Help
- **Decision**: Define segregated contracts in `Source/UI/IHelpSystem.h`:
  - `IHelpContentProvider`: Contract returning structured help metadata (`HelpTopic`) given a component or module identifier.
  - `IHelpOverlayHost`: Contract implemented by the top-level editor allowing child module panels to trigger contextual help popovers without tight coupling.
- **Rationale**: Keeps help text decoupled from UI layout code. New parameters or explanations can be edited in a central repository (`HelpContentRepository`) without touching graphics rendering code.

```text
[ModuleCardComponent] ──► [HelpButton ("?")]
          │                        │ (onClick)
          ▼                        ▼
[IHelpOverlayHost::showHelp(id, bounds)] ──► Queries [IHelpContentProvider]
                                                        │
                                                        ▼
                                            [HelpOverlayComponent] (Renders Card)
```

### 2. Custom LookAndFeel Aesthetic (`CamelPhatLookAndFeel`)
- **Color Palette**:
  - Background Base: `#12161a` (Deep Slate)
  - Card Panels: `#1a2026` with subtle `#2a323c` borders
  - Primary Accent: `#00e676` (Neon Emerald / Classic Camel Green)
  - Secondary Accent: `#ff9100` (Warm Amber Drive)
  - Modulation Accent: `#00e5ff` (Cyan Flange / Modulation)
  - Text: High-contrast `#f0f4f8`, muted labels `#8a99a8`
- **Rotary Knobs**:
  - Circular track with dynamic filled arc indicating current value.
  - Linear vertical drag feel (`juce::Slider::LinearVertical` velocity mapping).
  - Central indicator line and crisp numeric value readout below the dial.

### 3. Layout Hierarchy
- **Header**: Plugin brand title ("CamelPhat"), master bypass toggle, global preset selection, and global "?" help toggle.
- **Left Column**: BP Filter Card (Low Cut, High Cut, Resonance, BP Mix, Band-Reject Mix) and Magic EQ Card (Frequency, Amount).
- **Center Section**:
  - Dual X/Y Pads:
    - Pad 1 (Left): Filter Cutoff (X) vs Filter Resonance (Y).
    - Pad 2 (Right): Tube Drive (X) vs Mech Drive (Y).
  - Multi-Mode Filter Card: Cutoff, Resonance, Mode selector, Envelope Amount, Attack, Release.
  - Stereo Flanger Card: Rate, Depth, Feedback, Mix.
- **Right Column**: 4-Stage Distortion Card (Tube, Mech, Bit, Xcita, Mix) and Optical Compressor Card (Threshold, Ratio, Attack, Release, Phat Mode toggle, Comp Mix).
- **Master Section (Footer/Right)**: Master Dry/Wet Mix, Master Output Gain, and Stereo Animated VU Meter.

### 4. Interactive Dual X/Y Vector Control Pads
- **Component**: `XYPadComponent` inheriting from `juce::Component`.
- **Interaction**:
  - Crosshair grid with subtle glowing position puck.
  - Mouse drag updates both horizontal and vertical APVTS parameters simultaneously via atomic pointers or `juce::SliderAttachment` proxies.
  - Real-time visual feedback with coordinate tooltips.

### 5. 60 FPS Animated Stereo VU Meter
- **Component**: `StereoVUMeterComponent` inheriting from `juce::Component` and `juce::Timer`.
- **Timing**: `startTimerHz(60)` polls `processor.getMasterSection().getPeakLevel()` and `getRmsLevel()` on the message thread.
- **Safety**: Uses relaxed atomic reads; strictly zero memory allocation and zero contention with the audio thread.

## Risks / Trade-offs

- **[Risk: Popover clipping near window borders]** → *Mitigation:* `HelpOverlayComponent` dynamically computes popover placement (above, below, or side) based on target bounds and editor edges.
- **[Risk: UI redraw overhead lagging DAWs]** → *Mitigation:* Heavy background cards use static caching, rotary sliders repaint only within their local bounds, and VU meters repaint at fixed 60 FPS.
