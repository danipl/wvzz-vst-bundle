# Design

## Context

See proposal.md - Why. The plugin model is `Plugins/WvZzModulation/` (GH-15). That change added the custom center tab, the `ui/ModuleViewControls.h` helpers, the lock-free tap pattern (`dsp::LfoPhaseTap`), preset loading that resets to defaults, and explicit test checks. This change reuses all of them.

## Goals / Non-Goals

**Goals:**
- A delay line and a grain engine that WvZz Reverb (GH-17) can reuse without change.
- One PR. The ticket allows a split into two PRs; the design keeps Granular as a mode that plugs into the same loop, so the extra work is one engine class and its module.

**Non-Goals:**
- Multi-tap patterns, per-tap panning, reverse delay as its own mode.

## Decisions

### Shared DSP

1. **`dsp::DelayLine`** (`Source/Modules/Delay/DelayLine.h`, header-only). Power-of-two circular buffer, `prepare(maxDelaySamples)`, `push(x)`, `read(delaySamples)` with 3rd-order Lagrange interpolation, `clear()`. Convention: `read(d)` before `push(x[n])` returns `x[n - d]`; minimum delay 3 samples. It includes only the standard library.
2. **`GrainEngine`** (`Source/Modules/Grain/GrainEngine.h/.cpp`). A fixed `std::array` of 32 voices. Each voice reads a `DelayLine` at a delay that moves with the pitch ratio (`delay += 1 - ratio` per sample; reversed grains use `1 + ratio`), under a Hann window. A sample counter spawns grains at `grain_density`. A spawn is skipped when all 32 voices are busy. Random values come from an inline xorshift generator (no allocation). Output gain is `1 / sqrt(max(1, density x size))`.
3. **`GrainDSP`** is an `IDSPModule` with the `grain_*` parameters. Two ways to use it: (a) `renderGrains(lineL, lineR, baseDelayL, baseDelayR, outL, outR)` called by another module that owns the delay lines (the Delay module; later the Reverb shimmer); (b) stand-alone `process(ctx)`, which owns two 1-second lines and replaces the signal with its grains at a 100 ms base delay (a granular pitch shifter). `setExternalHost(true)` before `prepare` skips the stand-alone buffers.

### Delay module

4. **Loop per sample:** tap read → 2 all-pass diffusers → high-pass → low-pass → mode colour → this is the wet signal. Feedback write = `input x send + softclip(wet x feedback)`. Ping-Pong writes the mono input to the left line and crosses the feedback.
5. **Soft clipper:** `tanh`. The line input is bounded by `|input| + 1`, so 110% feedback cannot run away.
6. **Diffusion without a timing error.** The two all-pass sections per channel are always in the path, with gain `0.7 x dly_diffusion`. At gain 0 an all-pass is a pure delay of its length. The main tap is shortened by that fixed length (about 14 ms left, 16 ms right), so the total time is exact at any diffusion setting. This is why the shortest time is 20 ms. Alternative: switch the diffusers off at 0; rejected because switching clicks.
7. **Time changes.** Clean: two taps; a change starts a 50 ms linear crossfade from the old to the new delay; a change during a fade is taken when the fade ends. Tape and BBD: one tap whose delay follows the target through a one-pole glide (about 150 ms), so the pitch bends. Granular: new grains use the new time, running grains finish at the old one.
8. **Tape:** wow (0.5 Hz, up to ±1.5 ms) and flutter (6.3 Hz, up to ±0.15 ms) scaled by `dly_wow`; `tanh` drive scaled by `dly_drive` with gain compensation. **BBD:** an extra two-pole low-pass at `3500 Hz x sqrt(300 ms / time)`, limited to `1200–5000 Hz`, plus mild `tanh`, and half of the wow.
9. **Ducking:** peak follower on the dry input (5 ms attack, 250 ms release). Wet output gain = `1 - dly_duck x min(1, 4 x envelope)`. It acts on the output only, not inside the loop, so the repeats come back after the input stops.
10. **Bypass:** a smoothed `send` gain goes to 0 and the dry gain goes to 1. The loop keeps running, so the tail decays naturally.
11. **Parameters (15):** `bypass`, `mode`, `sync`, `time`, `sync_div` (14 note values from 1/1 to 1/32 with dotted and triplet), `routing`, `offset`, `feedback`, `hp`, `lp`, `diffusion`, `duck`, `wow`, `drive`, `mix`.
12. **Display data:** `DelayTapState` (`Source/Modules/Delay/DelayTapState.h`): atomics for left and right time in ms, feedback, mode, routing, duck gain, active flag. Written once per block.

### Plugin and UI

13. **Processor** like WvZz Modulation: stereo in/out; chain: input gain → modulation module → Delay (calls Grain in Granular mode) → Master. `getTailLengthSeconds()` = `time x ln(0.001) / ln(feedback)`, limited to `[time, 60]`; feedback ≥ 1 gives 60.
14. **Modulation profile `Delay`:** 10 appended `ModTarget` values (`DlyTime`, `DlyFeedback`, `DlyLp`, `DlyHp`, `DlyDiffusion`, `DlyWow`, `DlyMix`, `GrainSize`, `GrainPitch`, `GrainSpray`) plus the shared Master Mix, LFO 2 Rate, Master Vol.
15. **`ui::DelayTapDisplayComponent`** (`Source/Core/UI/`): reads `DelayTapState`; draws up to 16 repeats per channel on a time axis. Registered through `ModulationModule::setCustomDisplay("TAPS", ...)`.
16. **Layout (1080 x 680):** left x=10, w=320: Delay card (h 624). Center x=336, w=408. Right x=750, w=320: Grain (h 462), Master (h 156).

## File Changes

| File | Change |
| :--- | :--- |
| `Source/Modules/Delay/DelayLine.h`, `DelayTapState.h`, `DelayDSP.{h,cpp}`, `DelayView.{h,cpp}`, `DelayModule.h` | New |
| `Source/Modules/Grain/GrainEngine.{h,cpp}`, `GrainDSP.{h,cpp}`, `GrainView.{h,cpp}`, `GrainModule.h` | New |
| `Source/Core/UI/DelayTapDisplayComponent.{h,cpp}` | New |
| `Source/Core/Interfaces/ProcessContext.h`, `Source/Modules/Modulation/ModulationDSP.h` | `ModProfile::Delay`, 10 appended targets, names, mapping |
| `Source/Core/UI/HelpContentRepository.h` | Topics `delay`, `grain`, `delay_taps` |
| `Plugins/WvZzDelay/*` | New: processor, editor, presets, README, USER_MANUAL |
| `CMakeLists.txt`, `Makefile` | Sources, `wvzz_add_plugin(WvZzDelay WzDl "WvZz Delay")`, test sources, plugin list |
| `Tests/DSPTests.cpp` | New suites |
| `README.md`, `MANUAL.md`, `Plugins/WvZzDistortion/README.md` | Roadmap rows, plugin section, build table |

## Risks / Trade-offs

- [Feedback above 100% never decays] → The soft clipper bounds it; the 30-second test covers every mode; the tail is reported as 60 s.
- [Granular mode with pitch shift inside the feedback loop stacks the shift on every repeat] → This is the expected "shimmer ladder" sound; documented in the manual.
- [CPU: 32 voices x 2 channels of cubic interpolation] → Fixed upper bound; voices are skipped when idle.
- [UI cannot be tested automatically] → Editor construction test; display and controls checked in the Standalone app by screenshot. Listening still needs a person.

## Open Questions

- None that change the specs or the tasks.
