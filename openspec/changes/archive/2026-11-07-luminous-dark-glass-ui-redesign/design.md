# Design: Luminous Dark Glass Module Redesign with Unique WvZz Identity

## Context

See `proposal.md` for motivation and background. 

The WvZz plugin editor renders an 8-module suite across a 3-column layout at a base resolution of `1080 x 680`, resizable from `810 x 510` (75%) to `1920 x 1200` (200%). All controls and card chrome are currently drawn using JUCE vector primitives in `WvZzLookAndFeel` and `ModuleCardComponent`. While the layout is functional and real-time safe, the visual language relies on heavy 3D metallic dial gradients, solid card boundaries with horizontal lines, and only three shared neon colors across all eight modules.

This design establishes the technical strategy to modernize the LookAndFeel and module chrome to match the elegance of the modern studio grade suites while reinforcing WvZz's modular identity.

## Goals / Non-Goals

**Goals:**
- Implement a modern, minimalist "Hybrid Precision" UI styling inspired by modern studio-grade suites.
- Replace chunky metallic knobs with flat precision dark matte dials, rim tick indicators, and glowing progress tracks.
- Upgrade `ModuleCardComponent` to render dark translucent glass cards (`#151a22` to `#10141a`) with delicate hairline edge highlights, removing hard horizontal header dividing lines.
- Assign a dedicated, curated luminous accent color to each module to eliminate visual ambiguity and establish strong section identity.
- Provide responsive hover states that amplify arc glow and illuminate numeric values, with double-click inline text input.
- Maintain smooth 60 FPS rendering without dynamic memory allocations or software blur filters.

**Non-Goals:**
- Modifying DSP algorithms, parameter smoothing, or audio processing logic.
- Renaming or altering any APVTS parameter IDs.
- Changing the top-level 3-column layout structure, signal-flow indicator, or window dimensions.

## Decisions

### Decision 1: Two-Pass Vector Glow vs Software Blur Filters
- **Choice:** Draw glowing knob arcs using two successive vector path strokes:
  1. An outer wide stroke (width `trackWidth * 1.8f`, alpha `0.22f`) for ambient glow diffusion.
  2. An inner sharp stroke (width `trackWidth`, alpha `0.95f`) for luminous definition.
- **Why:** JUCE software blur filters (`juce::DropShadow` or bitmap image processing) consume substantial CPU and cannot be safely created inside paint routines without heap allocations. Two-pass vector strokes execute natively on vector rendering pipelines (Metal on macOS), scale cleanly to high-DPI displays without blur, and maintain zero heap allocations.
- **Alternatives considered:** Bitmap filmstrips (loss of vector resolution independence when resized); single 1px solid stroke (lacks the luminous multi-pass vector bloom).

### Decision 2: Translucent Dark Glass Cards with Top-Edge Highlight
- **Choice:** Paint `ModuleCardComponent` with a subtle vertical gradient (`#151a22` at top to `#10141a` at bottom) and a 1px border styled with a top-down light catch: the top edge and upper corners have alpha `0.35f` while side/bottom edges have alpha `0.15f`. The hard header divider line (`drawHorizontalLine(28)`) is eliminated, replacing it with 4px vertical breathing room and letter-spaced header typography.
- **Why:** Eliminates visual clutter across 8 simultaneous cards while keeping clear module spatial boundaries.
- **Alternatives considered:** Frameless floating pod controls (makes dense columns feel ungrounded); traditional beveled rack frames (looks dated and boxy).

### Decision 3: Flat Precision Rotary Dial & Perimeter Notch
- **Choice:** Replace the 3D-shaded metallic knob cap with a flat dark matte disc (`#181e26`) recessed into the card surface. Replace the center-to-edge needle line with an elegant perimeter tick or notch placed near the rim (length ~3.5px, width ~2.0px). Sliders receive a slim track (4.0px) with a sleek rounded capsule thumb fader.
- **Why:** Captures precision vector ergonomics, avoids center-pivot visual noise, and provides intuitive angular reading.
- **Alternatives considered:** Dial-less ring with floating text (harder to click and drag at small scales).

### Decision 4: Curated Luminous Module Palette
- **Choice:** Base canvas uses deep obsidian charcoal (`#0d1015`). Each module is assigned a dedicated luminous accent color:
  - **Distortion:** Vibrant Ember / Coral Red (`#ff4545`)
  - **Magic EQ:** Electric Sky Cyan (`#00e5ff`)
  - **Compressor:** Warm Honey / Ochre Gold (`#ffb300`)
  - **MM Filter:** Vivid Emerald Green (`#00e676`)
  - **Flanger:** Electric Violet (`#b388ff`)
  - **BP Filter:** Warm Brass / Amber (`#ff9100`)
  - **Modulation / XY:** Dual-Tone Cyan & Tangerine (`#00e5ff` & `#ff9100`)
  - **Master:** Arctic Ice White (`#e0f7fa`)
- **Why:** Gives immediate at-a-glance recognition for each module, mirroring curated luminous color coding.
- **Alternatives considered:** Monochrome blue/slate across all modules (harder to distinguish functional blocks quickly).

### Decision 5: Interactive Hover Glow & Double-Click Text Entry
- **Choice:** Inspect `slider.isMouseOverOrDragging()` inside `drawRotarySlider`:
  - When idle: Outer arc glow is subdued, and numeric value text is rendered in muted silver (`#8a99a8`).
  - When hovered/dragged: Outer arc glow expands and intensifies in the module's accent color, and the numeric text brightens to high-contrast white (`#ffffff`).
  - Enable inline text entry on double-click by configuring slider text box behavior.
- **Why:** Delivers immediate tactile feedback and enables precision direct typing without permanently cluttering the interface.

## Risks / Trade-offs

- **[Risk] High GUI CPU load during animated visualizer redraws** → *Mitigation:* Glow strokes use pre-computed circular paths and standard path strokes; no expensive image filters or dynamic allocations are introduced in `paint()`.
- **[Risk] Legibility at 75% minimum window scale (`810 x 510`)** → *Mitigation:* Enforce a minimum clamp of 2.0px for track strokes and 9.0pt for font rendering to prevent illegibility at minimum window bounds.

## Open Questions

- **Custom Font Asset Bundling:** Should we bundle a custom geometric font (e.g., Inter or Roboto) or rely on JUCE's default modern sans-serif engine?  
  *Resolution:* Deferred to implementation. Default JUCE sans-serif rendering will be used first; custom font embedding can be introduced cleanly later without changing LookAndFeel APIs.
