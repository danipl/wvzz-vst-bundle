# Design

## Context

See proposal.md - Why. The test binary `wvzz_dsp_tests` is built by `make test` with `CMAKE_BUILD_TYPE=Release`, which defines `NDEBUG`.

## Goals / Non-Goals

**Goals:**
- Old and new suites fail loudly on a wrong result, without rewriting 62 call sites.
- Fix what the real checks reveal.

**Non-Goals:**
- A test framework. Refactoring the old suites.

## Decisions

1. **Redefine `assert` in the test file.** After the includes, `#undef assert` and define a macro that prints the condition, file, and line and calls `std::exit(1)`. The existing `assert(x && "message")` calls keep working. Alternative: build the tests in Debug; rejected because CI and `make test` verify the Release code that ships.
2. **Test setup defects.** A DSP class without bound parameters has no storage for its switches, and its smoothers start at 0. `testBandRejectRecombination` now calls `setCutoffs(200, 5000)`. The Magic EQ pre-distortion check uses `WvZzDistortionAudioProcessor` and the `meq_pre` parameter.
3. **Mono maker on mid/side.** Old code: per channel `x - lowpass(x) + mono(lowpass)`. The low-pass shifts the phase, so `x - lowpass(x)` kept about 73% of a 60 Hz tone. New code: `mid = (L+R)/2` passes unchanged; `side = (L-R)/2` goes through two 12 dB/oct high-pass stages at 120 Hz (the two existing SVF instances); `L = mid + side`, `R = mid - side`. Mono content is bit-exact apart from float rounding. Alternative: Linkwitz-Riley split per channel; rejected, it changes the phase of mono content.
4. **Dynamics presets** reset all parameters to default before applying, like WvZz Distortion and the three new plugins already do.
5. **Dynamics DICE** uses a table of `{parameter id, low, high}` in real units and `setValueNotifyingHost`, like WvZz Distortion. The module `randomize()` calls are removed from this plugin's DICE path: they only acted while a smoother was moving and bypassed the host.
6. **Preset index in state.** `getStateInformation` adds the property `currentProgram` to the state tree; `setStateInformation` reads it with default 0 and clamps it. WvZz Delay and WvZz Reverb already do this.

## File Changes

| File | Change |
| :--- | :--- |
| `Tests/DSPTests.cpp` | `assert` macro, 2 test setup fixes, `testMasterMonoBass`, `testPresetStateAndDice` |
| `Source/Modules/Master/MasterDSP.{h,cpp}` | Mid/side sub-bass mono maker |
| `Plugins/WvZzDynamics/PluginProcessor.cpp` | Preset reset, DICE through host parameters, preset index in state |
| `Plugins/WvZzDistortion/PluginProcessor.cpp`, `Plugins/WvZzModulation/PluginProcessor.cpp` | Preset index in state |
| `Plugins/*/USER_MANUAL.md` (Dynamics, Modulation, Delay, Reverb) | Presets, DICE, mono bass wording |

## Risks / Trade-offs

- [The mono maker changes the sound of sessions that use it] → It is the documented behavior ("sums frequencies below 120 Hz to mono"); the old code did not do it. Named in the PR.
- [Dynamics presets now reset parameters the user set by hand] → Same behavior as the other four plugins; documented in the manual.
