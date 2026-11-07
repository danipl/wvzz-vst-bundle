# Tasks: Luminous Dark Glass Module Redesign with Unique WvZz Identity

## 1. Foundation & Color Palette Modernization

- [x] 1.1 Update `WvZzLookAndFeel` palette definitions in `Source/Core/UI/WvZzLookAndFeel.h` and `.cpp` with the deep obsidian base (`#0d1015`), dark slate glass panel tones (`#151a22`), and dedicated module accent constants (`coralRed`, `skyCyan`, `honeyGold`, `emeraldGreen`, `electricViolet`, `warmAmber`, `arcticIce`). Verify compilation with `make build`.
- [x] 1.2 Refactor `ModuleCardComponent` in `Source/Core/UI/ModuleCardComponent.h` to render a vertical dark glass gradient fill, top-edge hairline highlight, remove the hard horizontal divider line, and apply clean uppercase title typography. Verify bounds and header alignment.

## 2. Sleek Precision Controls in LookAndFeel

- [x] 2.1 Overhaul `WvZzLookAndFeel::drawRotarySlider` to replace 3D metallic discs with flat dark matte caps, perimeter rim tick pointers, and two-pass vector glowing arcs (unipolar and bipolar).
- [x] 2.2 Add interactive hover/drag glow amplification and high-contrast value text illumination to `drawRotarySlider`, with double-click numeric entry enabled.
- [x] 2.3 Modernize `WvZzLookAndFeel::drawLinearSlider` with slim 4.0px tracks, glowing active fill, and minimalist capsule fader thumbs.
- [x] 2.4 Polish toggle pill buttons, combo boxes, and popup menus in `WvZzLookAndFeel` to match the refined dark slate glass aesthetic.

## 3. Module Views & Palette Assignment

- [x] 3.1 Update `PluginEditor` in `Source/PluginEditor.cpp` and `.h` to assign the curated luminous colors to each module card (Distortion: Coral Red, Magic EQ: Sky Cyan, Compressor: Honey Gold, MM Filter: Emerald Green, Flanger: Violet, BP Filter: Amber, Modulation/XY: Cyan & Tangerine, Master: Arctic Ice).
- [x] 3.2 Update module views in `Source/Modules/` (`DistortionView`, `MagicEQView`, `CompressorView`, `MMFilterView`, `FlangerView`, `BPFilterView`, `ModulationView`, `MasterView`) to apply matching module accents to rotary dials and toggle buttons.
- [x] 3.3 Harmonize the BP Filter passband visualizer, LFO waveform scopes, and stereo VU meters with the new palette and dark obsidian backdrop.

## 4. Verification & Quality Gate

- [x] 4.1 Run `make build` and verify macOS Universal Binary (`arm64;x86_64`) compilation succeeds with zero warnings.
- [x] 4.2 Run `make test` and verify automated zero-allocation DSP tests (`wvzz_dsp_tests`) pass with zero heap allocations.
- [x] 4.3 Validate the change with `openspec validate --all` to ensure all delta and main specifications pass strict validation.

## Workflow follow-up

- Archive the change (`openspec archive luminous-dark-glass-ui-redesign`) after implementation and review approval.
