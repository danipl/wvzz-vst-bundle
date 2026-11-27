# Design

## Context

See proposal.md - Why. The plugin model is `Plugins/WvZzDelay/` (GH-16). This change reuses `dsp::DelayLine`, `dsp::GrainEngine`, the custom center tab, the `ui/ModuleViewControls.h` helpers, the lock-free state pattern, preset loading with reset, and the explicit test checks.

## Goals / Non-Goals

**Goals:**
- A decay time that matches the Decay knob (10% tolerance) in Plate and Hall mode.
- A loop that cannot run away, also with shimmer at maximum.

**Non-Goals:**
- Room simulation by geometry. Gated, reverse, or convolution reverb. Freeze.

## Decisions

### Early reflections

1. **Bus use.** `EarlyReflectionsDSP` reads the main buffer and writes its result, scaled by `er_level`, to the auxiliary bus (`ctx.auxBandRejectBuffer`), and sets `ctx.hasAuxSplit`. It does not change the main buffer. The Reverb module then uses `main + aux` as its input and adds `aux` to its wet signal. When the module is bypassed it does not set the flag. Alternative: mix the early reflections into the main buffer; rejected because the Reverb module needs the clean dry signal for its dry/wet mix.
2. **Pattern.** Per channel one `DelayLine` (0.12 s) with 8 taps at fixed fractions of the pattern length (`16 ms + 64 ms x er_size`), with falling gains and mixed signs; left and right use different fractions. Two all-pass sections follow, with gain `0.6 x er_diffusion`.

### Reverb

3. **Topology.** Dry + early reflections → pre-delay (`DelayLine`, 0.3 s) → 4 input all-pass diffusers per channel → FDN with 8 `DelayLine`s. Left feeds the even lines, right the odd lines. Outputs: left = signed sum of the even lines, right = signed sum of the odd lines.
4. **Mixing matrix:** 8 x 8 Hadamard, computed in place with 3 butterfly stages and scaled by `1/sqrt(8)`. It is orthogonal, so it preserves energy. Alternative: Householder (cheaper, less mixing); rejected, the Hadamard matrix gives a denser tail.
5. **Decay.** Each line has the gain `10^(-3 x length / (decay x sampleRate))`. With an orthogonal matrix every mode of the network then decays by 60 dB in `rev_decay` seconds. All gains are below 1 for every setting (0.9995 at 20 s with the longest line).
6. **Line lengths.** Base lengths in ms. Plate: 11.3, 14.9, 18.1, 21.7, 25.3, 29.9, 34.1, 38.3. Hall and Shimmer: 37.3, 43.7, 51.1, 59.3, 67.9, 76.1, 85.7, 97.3. `rev_size` scales them by `0.5 .. 1.5`. A size change glides (one-pole, about 100 ms), so it bends the pitch for a moment instead of clicking. A mode change crossfades the wet output down and up around a network clear (about 30 ms each way).
7. **Damping.** In each line: a one-pole low-pass at `rev_hidamp` and a one-pole high-pass at `rev_lodamp`. With both fully open (20 kHz, 20 Hz) the extra loss at 1 kHz is below 1%, so the Decay knob is exact there. The states are flushed against denormals.
8. **Loudness.** The feed into the network is scaled by `sqrt(1 - g^2)` (g = mean line gain), so a long decay does not get louder than a short one, and a constant loud input cannot pile up without limit.
9. **Tail modulation.** Four of the eight lines are read with a slow sine offset (0.31 to 0.97 Hz, up to ±12 samples x `rev_mod`) through the fractional read.
10. **Shimmer.** The stereo network output is written into two `DelayLine`s (0.5 s). A `dsp::GrainEngine` reads them at a 120 ms base delay with the pitch ratio of `rev_interval` (`+12`, `+7`, `+19`, `+24`, `-12` semitones). Its output goes through a 9 kHz low-pass and `tanh`, is scaled by `0.5 x rev_shimmer`, and is added to the network input. The `tanh` bounds what the pitch path can add, so the loop stays stable. In Plate and Hall mode the path is off.
11. **Width.** Mid/side: `side x rev_width`.
12. **Bypass and mix** like the Delay module: a smoothed `send` goes to 0, the dry gain goes to 1, the tail keeps decaying. `rev_mix` 0 gives exactly the input.
13. **Parameters (13):** `bypass`, `mode`, `predelay`, `size`, `decay`, `lodamp`, `hidamp`, `diffusion`, `mod`, `width`, `shimmer`, `interval`, `mix`. Early reflections (4): `bypass`, `size`, `diffusion`, `level`.
14. **Display data:** `ReverbDecayState` (`Source/Modules/Reverb/ReverbDecayState.h`): atomics for decay, pre-delay, damping frequencies, mean line length, mode, shimmer, wet level, active flag.

### Plugin and UI

15. **Processor** like WvZz Delay. Chain: input gain → modulation module → EarlyReflections → Reverb → Master. Tail = pre-delay + decay + 0.1 s. The selected preset index is saved with the state.
16. **Modulation profile `Reverb`:** 11 appended `ModTarget` values plus Master Mix, LFO 2 Rate, Master Vol.
17. **`ui::ReverbDecayDisplayComponent`**: reads `ReverbDecayState`; a pure function `decayDbAt(timeSec, ...)` gives the curve and is unit-tested. The low and high lines use the extra loss per pass of the damping filters at 100 Hz and 8 kHz.
18. **Layout (1080 x 680):** left x=10, w=320: Reverb card (h 624). Center x=336, w=408. Right x=750, w=320: Early Reflections (h 462), Master (h 156).

## File Changes

| File | Change |
| :--- | :--- |
| `Source/Modules/EarlyReflections/EarlyReflections{DSP,View}.{h,cpp}`, `EarlyReflectionsModule.h` | New |
| `Source/Modules/Reverb/Reverb{DSP,View}.{h,cpp}`, `ReverbModule.h`, `ReverbDecayState.h` | New |
| `Source/Core/UI/ReverbDecayDisplayComponent.{h,cpp}` | New |
| `Source/Core/Interfaces/ProcessContext.h`, `Source/Modules/Modulation/ModulationDSP.h` | `ModProfile::Reverb`, 11 appended targets |
| `Source/Core/UI/HelpContentRepository.h` | Topics `reverb`, `early_reflections`, `reverb_decay` |
| `Plugins/WvZzReverb/*` | New: processor, editor, presets, README, USER_MANUAL |
| `CMakeLists.txt`, `Makefile` | Sources, `wvzz_add_plugin(WvZzReverb WzRv "WvZz Reverb")`, test sources, plugin list |
| `Tests/DSPTests.cpp` | New suites |
| `README.md`, `MANUAL.md`, `Plugins/WvZzDistortion/README.md` | All five plugins shown as available |

## Risks / Trade-offs

- [Damping changes the decay time] → By design: Decay is the mid-frequency time with the filters open; the display shows the faster low and high lines.
- [Shimmer in the loop] → `tanh` and a low-pass on the pitch path; the 30-second test at maximum decay and shimmer checks the bound and that the level stops growing.
- [Loudness scaling makes short decays relatively louder] → Accepted; it keeps the wet level steady across the Decay range.
- [UI cannot be tested automatically] → Editor construction test and a screenshot check of the Standalone app. Listening still needs a person.

## Open Questions

- None that change the specs or the tasks.
