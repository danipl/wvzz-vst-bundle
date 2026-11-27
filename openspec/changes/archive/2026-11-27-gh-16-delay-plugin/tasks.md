# Tasks

## 1. Shared DSP

- [x] 1.1 Add `dsp::DelayLine` and a test that `read(d)` returns the sample pushed `d` samples before, for integer and fractional `d`; verify it passes
- [x] 1.2 Add `ModProfile::Delay`, the 10 appended `ModTarget` values, names, and mapping; verify the target-list tests of the three existing profiles still pass

## 2. Grain module

- [x] 2.1 Implement `GrainEngine` and `GrainDSP` (parameters, `renderGrains`, stand-alone `process`); add tests for pitch shift +12, the 32-voice limit, zero allocation, and prefixing; verify they pass
- [x] 2.2 Implement `GrainView` and `GrainModule`, add the help topic `grain`; verify the module compiles into `WvZzModules`

## 3. Delay module

- [x] 3.1 Implement `DelayDSP` (time, sync, routing, feedback loop with filters and soft clipper, diffusion, ducking, bypass tail, four modes, `DelayTapState`); verify it compiles
- [x] 3.2 Add tests: impulse timing, tempo sync, 4-second maximum, ping-pong sides, bounded output at 110% feedback for 30 s in each mode, low-pass darkening, diffusion spread; verify they pass
- [x] 3.3 Add tests: Clean and Granular time-change crossfade without click, Tape and BBD pitch bend, BBD darker than Clean, tape wow, ducking, bypass tail and transparency, granular delay timing, zero allocation in every mode, prefixing; verify they pass
- [x] 3.4 Implement `DelayView` and `DelayModule`, add the help topic `delay`; verify the module compiles into `WvZzModules`

## 4. WvZz Delay plugin

- [x] 4.1 Create the processor (chain, dynamic tail length, preset loading with reset), add `wvzz_add_plugin(WvZzDelay WzDl "WvZz Delay")`, the Makefile entry, and the test sources; verify `make build` succeeds
- [x] 4.2 Write 10 factory presets; add tests for recall, stability, history independence, tail length, LFO delivery to Feedback, and plugin zero allocation; verify they pass
- [x] 4.3 Add `ui::DelayTapDisplayComponent` and the editor with the `TAPS` tab; add the editor test (1080 x 680, one tap display, tap state published); verify it passes

## 5. Docs

- [x] 5.1 Write `Plugins/WvZzDelay/README.md` and `USER_MANUAL.md`; verify every parameter id, range, and default matches the code and all 10 presets are listed
- [x] 5.2 Update the root `README.md` (roadmap row, plugin section, toolbox list), the Distortion README row, and the `MANUAL.md` build table; verify no "Planned" remains for WvZz Delay and the version badge line is unchanged

## 6. Integration check

- [x] 6.1 Run `make package`; verify build and tests pass and 4 VST3 bundles + 4 Standalone apps are Universal Binary with valid signatures; plugin code `WzDl` is in the generated build
- [x] 6.2 Run the test suites in an AddressSanitizer build; verify no memory error is reported
- [x] 6.3 Run `openspec validate --all`; verify it passes
- [x] 6.4 Launch `WvZz Delay.app`; verify by screenshot that the cards, the three center tabs, and the tap display render

## Workflow follow-up

- Archive this change before the PR merges.
- PR title: `feat(delay): add WvZz Delay plugin (clean, tape, BBD, granular)`, with `Closes #16`.
- The DoD item "checked by hand in the Standalone app" needs a person to listen and turn the knobs.
