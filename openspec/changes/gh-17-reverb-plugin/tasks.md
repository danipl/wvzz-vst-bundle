# Tasks

## 1. Shared pieces

- [ ] 1.1 Add `ModProfile::Reverb`, the 11 appended `ModTarget` values, names, and mapping; verify the target-list tests of the four existing profiles still pass

## 2. Early reflections module

- [ ] 2.1 Implement `EarlyReflectionsDSP`, its view and module, and the help topic; add tests for its 3 scenarios, zero allocation, and prefixing; verify they pass

## 3. Reverb module

- [ ] 3.1 Implement `ReverbDSP` (pre-delay, input diffusion, 8-line FDN with Hadamard matrix, per-line decay gains, damping, modulation, shimmer path, width, mix, bypass tail, `ReverbDecayState`); verify it compiles
- [ ] 3.2 Add tests: matrix energy, measured decay within 10% in Plate and Hall for two decay values and two sizes, bounded output for 30 s at maximum decay and shimmer, mix 0 equals input; verify they pass
- [ ] 3.3 Add tests: Hall builds up later than Plate, shimmer octave energy, pre-delay silence, high damping, width, bypass tail, zero allocation in each mode, prefixing; verify they pass
- [ ] 3.4 Implement `ReverbView` and `ReverbModule`, add the help topic; verify the module compiles into `WvZzModules`

## 4. WvZz Reverb plugin

- [ ] 4.1 Create the processor (chain, tail length, preset loading with reset, saved preset index), add `wvzz_add_plugin(WvZzReverb WzRv "WvZz Reverb")`, the Makefile entry, and the test sources; verify `make build` succeeds
- [ ] 4.2 Write 10 factory presets; add tests for recall, stability, history independence, state recall, tail length, LFO delivery to Reverb Mix, and plugin zero allocation; verify they pass
- [ ] 4.3 Add `ui::ReverbDecayDisplayComponent` and the editor with the `DECAY` tab; add the editor test (1080 x 680, one decay display, decay state published, curve function); verify it passes

## 5. Docs

- [ ] 5.1 Write `Plugins/WvZzReverb/README.md` and `USER_MANUAL.md`; verify every parameter id, range, and default matches the code and all 10 presets are listed
- [ ] 5.2 Update the root `README.md` (roadmap row, plugin section, toolbox list), the Distortion README row, and the `MANUAL.md` build table; verify `grep -i planned README.md` finds no plugin row and the version badge line is unchanged

## 6. Integration check

- [ ] 6.1 Run `make package`; verify build and tests pass and 5 VST3 bundles + 5 Standalone apps are Universal Binary with valid signatures; plugin code `WzRv` is in the generated build
- [ ] 6.2 Run the test suites in an AddressSanitizer build; verify no memory error is reported
- [ ] 6.3 Run `openspec validate --all`; verify it passes
- [ ] 6.4 Launch `WvZz Reverb.app`; verify by screenshot that the cards, the three center tabs, and the decay display render

## Workflow follow-up

- Archive this change before the PR merges.
- PR title: `feat(reverb): add WvZz Reverb plugin (plate, hall, shimmer)`, with `Closes #17`.
- After the merge, check the epic GH-12 Definition of Done.
- The DoD item "checked by hand in the Standalone app" needs a person to listen and turn the knobs.
