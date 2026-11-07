# Proposal: Luminous Dark Glass Module Redesign with Unique WvZz Identity

## Why

The current WvZz module interface uses heavy 3D metallic dial renderings, solid boxy module borders with horizontal dividing lines, and a repetitive three-color neon palette. While functional, it feels visually cluttered and dated compared to the clean, luminous, and refined user experience musicians expect from modern industry standards like the modern studio grade suites.

Redesigning the interface introduces flat precision controls, subtle translucent dark glass cards, multi-pass vector glowing arcs, and dedicated module accent color identities. This eliminates visual friction, improves readability during high-density sound design sessions, and establishes a distinctive, premium visual identity for WvZz while preserving its modular rack workflow.

## What Changes

- **Sleek Vector Controls**: Replace 3D metallic rotary dials with flat dark matte caps, perimeter tick pointers, and two-pass glowing outer progress arcs. Add flat capsule fader thumbs and slim tracks for linear sliders.
- **Dynamic Interaction & Value Display**: Implement responsive hover glow amplification on all knobs and sliders. Replace static gray value strings with high-contrast text that illuminates on hover/drag, and enable inline direct keyboard text input on double-click.
- **Translucent Dark Glass Chrome**: Update `ModuleCardComponent` to use subtle dark slate gradient backgrounds (`#151a22` to `#10141a`), remove hard horizontal header divider lines, add delicate top-edge hairline highlights, and render clean letter-spaced uppercase header titles.
- **Curated Luminous Module Palette**: Shift base canvas to deep obsidian charcoal (`#0d1015`) and assign dedicated signature luminous accents per module (Distortion: Coral/Ember, Magic EQ: Sky Cyan, Compressor: Honey Gold, MM Filter: Emerald Green, Flanger: Violet, BP Filter: Amber, Modulation/XY: Dual Cyan/Tangerine, Master: Ice White).
- **High-Performance 60 FPS Rendering**: Ensure all glow effects use pure two-pass vector path strokes rather than costly software blur filters, maintaining smooth 60 FPS rendering across 75% (`810 x 510`) to 200% (`1920 x 1200`) scaling.

## Capabilities

### New Capabilities
*(None)*

### Modified Capabilities
- `plugin-ui-system`: Updates visual styling requirements for rotary dials (flat cap with perimeter tick pointer and two-pass vector glow), module card chrome (translucent glass with hairline highlight, no hard divider line), hover value readouts, and the curated per-module luminous color system.

## Impact

- **Affected Source Files**:
  - `Source/Core/UI/WvZzLookAndFeel.h` & `Source/Core/UI/WvZzLookAndFeel.cpp`
  - `Source/Core/UI/ModuleCardComponent.h`
  - `Source/PluginEditor.h` & `Source/PluginEditor.cpp`
  - `Source/Modules/*/*View.h` & `Source/Modules/*/*View.cpp` (accent colors and knob/label layout)
- **APIs & State**: Zero changes to APVTS parameter IDs, parameter normalization, DSP audio processing, or thread safety invariants.
- **Target Formats**: Preserves macOS Universal Binary (`arm64;x86_64`) VST3 and Standalone targets.
