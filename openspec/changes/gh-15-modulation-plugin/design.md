# Design

## Context

See proposal.md - Why. The model for a plugin is `Plugins/WvZzDynamics/`; the model for a module is `Source/Modules/Flanger/` (DSP class, module class, view class). The build uses `wvzz_add_plugin()` from GH-14.

## Goals / Non-Goals

**Goals:**
- Three new headless DSP modules and an extended Flanger, all real-time safe.
- A reusable way to add a plugin-specific display tab (WvZz Delay and WvZz Reverb need it next).
- No change in sound or saved state for WvZz Distortion and WvZz Dynamics.

**Non-Goals:**
- Tempo sync inside the four effects. Reordering effects.

## Decisions

### DSP

1. **Chorus.** One circular buffer per channel (50 ms, allocated in `prepare`), read by up to 4 voices with 3rd-order Lagrange interpolation. Voice `k` of `N` has LFO phase `k/N` of a cycle. The right channel adds `chor_spread x 90°`. Wet sum is scaled by `1/sqrt(N)`. Feedback takes the first voice, through `tanh`, into the buffer write. Alternative considered: one delay line per voice; rejected because all voices read the same input, so one buffer is enough.
2. **Phaser.** Per channel, up to 12 first-order all-pass sections `y = a*x + x1 - a*y1`, `a = (tan(pi*fc/fs) - 1) / (tan(pi*fc/fs) + 1)`. `fc = phs_center * 2^(2 * phs_depth * lfo)`, limited to `20 Hz .. 0.45 fs`. The coefficient is updated every sample. Feedback adds the last cascade output, limited by `tanh`, to the input. All 12 sections always exist; `phs_stages` selects how many run. Alternative: ZDF SVF all-pass (second order); rejected, first-order sections give the classic sound at lower cost.
3. **Tremolo.** Classic: `gain = 1 - depth * 0.5 * (1 - lfo)`. Harmonic: split with a one-pole low-pass at `trem_xover`; `high = input - low` (the two bands sum to the input exactly); low gain uses `+lfo`, high gain uses `-lfo`. The square shape is smoothed by a 2 ms one-pole to avoid clicks.
4. **Flanger.** The three fixed constants become smoothed parameters with the same defaults. The delay is limited to at least 1 sample. Through-zero adds a second, dry-only circular buffer; the dry path reads it at `flg_center`. With `flg_tzero` off the code path and arithmetic match the old code. The equivalence test keeps a copy of the old algorithm inside `Tests/DSPTests.cpp` as the reference.
5. **Denormals.** Every feedback state is flushed with a `1e-15` threshold check, in addition to `ScopedNoDenormals` in `processBlock`.
6. **LFO state for the UI.** New `dsp::LfoPhaseTap` (`Source/Core/DSP/LfoPhaseTap.h`): a struct of `std::atomic` values (phase 0..1, stereo offset, shape, active flag). Each of the four effect DSP classes owns one and stores to it once per block. It has no GUI dependency.

### Modulation targets

7. **Append, never reorder.** `ModTarget` gets 9 new values after `DynClipDrive`; `ModulationDSP::numTargets` grows from 34 to 43. `ModProfile::Modulation` maps 15 list entries (None + 14 targets) to enum values, like the Dynamics profile. Flanger keeps `FlangerRate` / `FlangerAmount`.

### Plugin

8. **Processor** follows WvZz Dynamics: stereo in/out, no sidechain bus. Chain: input gain → save dry → modulation module → chorus → phaser → flanger → tremolo → master. `getTailLengthSeconds()` returns `2.0`.
9. **Presets.** `setCurrentProgram` first resets every effect, modulation, and master parameter to its default, then applies the preset values. This makes a preset independent of the one loaded before (WvZz Dynamics has that defect; fixing it there is out of scope). Program 0 is applied in the constructor so the plugin does not open with every effect bypassed.

### UI

10. **Custom display tab.** `ModulationModule::setCustomDisplay(title, factory)` stores a tab title and a `std::function<std::unique_ptr<juce::Component>()>`. `ModulationView` creates the component, adds a third tab button, and switches between three states (X/Y, monitor, custom). Without a custom display the header layout is byte-for-byte the old one. The view calls `repaint()` on the custom component from `updateVisuals()` (60 Hz editor timer).
11. **`ui::LfoPhaseScopeComponent`** (`Source/Core/UI/`): constructed with a list of `{name, colour, const dsp::LfoPhaseTap*}`. It only reads atomics.
12. **Shared view helpers.** New header `Source/Core/UI/ModuleViewControls.h` with `RotaryControl`, `ComboControl`, `ToggleControl` and their init / layout functions. The new views use it. Existing views are not refactored.
13. **Flanger view.** `FlangerModule` and `FlangerView` take a flag `extendedControls` (default `false`). WvZz Modulation passes `true` and gets Feedback, Depth, Center knobs and a `TZ` toggle. WvZz Distortion keeps its 2-knob card.
14. **Layout (1080 x 680).** Left column x=10, w=320: Chorus (h 309), Phaser (h 309). Center x=336, w=408: modulation card. Right column x=750, w=320: Flanger (h 228), Tremolo (h 228), Master (h 156).

## File Changes

| File | Change |
| :--- | :--- |
| `Source/Core/Interfaces/ProcessContext.h` | `ModProfile::Modulation`, 9 appended `ModTarget` values |
| `Source/Core/DSP/LfoPhaseTap.h` | New |
| `Source/Core/UI/ModuleViewControls.h`, `LfoPhaseScopeComponent.{h,cpp}` | New |
| `Source/Core/UI/HelpContentRepository.h` | Topics `chorus`, `phaser`, `tremolo`; updated `flanger` |
| `Source/Modules/Chorus/`, `Phaser/`, `Tremolo/` | New (5 files each) |
| `Source/Modules/Flanger/*` | 4 parameters, through-zero, extended view flag, LFO tap |
| `Source/Modules/Modulation/*` | Profile names / mapping, `numTargets`, custom tab |
| `Plugins/WvZzModulation/*` | New: processor, editor, presets, README, USER_MANUAL |
| `CMakeLists.txt`, `Makefile` | Module sources, `wvzz_add_plugin(WvZzModulation WzMd "WvZz Modulation")`, test sources, plugin list |
| `Tests/DSPTests.cpp` | New suites |
| `README.md`, `MANUAL.md`, `Plugins/WvZzDistortion/README.md` | Roadmap rows, build table |

## Risks / Trade-offs

- [Flanger change alters Distortion sound] → Equivalence test against the embedded old algorithm, plus the existing Distortion preset test.
- [Growing `numTargets` breaks a module that assumed 34] → Offsets are looked up by enum value with bounds checks; existing tests for Distortion and Dynamics modulation still run.
- [Through-zero adds `flg_center` of delay to the dry path] → Only when the switch is on; documented in the manual. No latency is reported, because the effect is off by default and the delay changes with a knob.
- [UI cannot be tested automatically] → Editor construction test at 1080 x 680; the scope and controls are checked by hand in the Standalone app (see tasks, last group).

## Open Questions

- None that change the specs or the tasks.
