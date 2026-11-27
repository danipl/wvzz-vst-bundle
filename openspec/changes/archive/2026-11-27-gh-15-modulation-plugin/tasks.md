# Tasks

## 1. Flanger extension (DSP)

- [x] 1.1 Add `flg_feedback`, `flg_depth`, `flg_center`, `flg_tzero` and the LFO tap to `FlangerDSP`; add a test with the old algorithm embedded as reference and verify default output matches it within `1e-5` (scenario "Default values keep the previous sound")
- [x] 1.2 Add tests for through-zero and negative feedback; verify both pass and the existing Distortion preset test still passes

## 2. Shared pieces

- [x] 2.1 Add `dsp::LfoPhaseTap` and the `Modulation` profile (`ModProfile`, 9 `ModTarget` values, names, index mapping, `numTargets` 43); verify the existing modulation target tests for Distortion and Dynamics pass unchanged
- [x] 2.2 Add `ui/ModuleViewControls.h`; verify it compiles in `WvZzModules`

## 3. New DSP modules

- [x] 3.1 Implement `Chorus` (DSP, module, view) and add it to `CMakeLists.txt`; add tests for its 3 scenarios, zero allocation, and prefixing; verify they pass
- [x] 3.2 Implement `Phaser` (DSP, module, view); add tests for its 3 scenarios, zero allocation, and prefixing; verify they pass
- [x] 3.3 Implement `Tremolo` (DSP, module, view); add tests for its 4 scenarios, zero allocation, and prefixing; verify they pass
- [x] 3.4 Add help topics `chorus`, `phaser`, `tremolo` and update `flanger` in `HelpContentRepository.h`; verify each new view's help button uses an existing topic id

## 4. Custom display tab and LFO phase scope

- [x] 4.1 Add `setCustomDisplay` to `ModulationModule` and the third tab to `ModulationView`; verify the Distortion and Dynamics editor tests pass (two-tab layout unchanged)
- [x] 4.2 Add `ui::LfoPhaseScopeComponent`; verify it builds and paints from `LfoPhaseTap` values only (no DSP class include)
- [x] 4.3 Add the `extendedControls` flag to `FlangerModule` / `FlangerView`; verify the Distortion editor test passes

## 5. WvZz Modulation plugin

- [x] 5.1 Create `Plugins/WvZzModulation/` processor with the fixed chain, tail length `2.0`, and preset loading that resets to defaults first; add `wvzz_add_plugin(WvZzModulation WzMd "WvZz Modulation")`, the Makefile list entry, and the test target sources; verify `make build` succeeds
- [x] 5.2 Write 10 factory presets in `Presets/FactoryPresets.h`; add tests for preset recall, stability, and history independence; verify they pass
- [x] 5.3 Create the editor (shared shell, 5 cards + center column, scope tab); add the editor construction test at `1080 x 680`; verify it passes
- [x] 5.4 Add plugin tests: all-bypassed output equals input, effects stack in series, zero allocation over 1,000 blocks, tail length `2.0`, LFO target delivery for Chorus Mix; verify they pass

## 6. Docs

- [x] 6.1 Write `Plugins/WvZzModulation/README.md` and `USER_MANUAL.md`; verify every parameter id, range, and default matches the code and all 10 presets are listed
- [x] 6.2 Update the root `README.md` (roadmap row + plugin section), the Distortion README roadmap row, and the `MANUAL.md` build table; verify no "Planned" remains for WvZz Modulation and the version badge line is unchanged

## 7. Integration check

- [x] 7.1 Run `make package`; verify build and tests pass and 3 VST3 bundles + 3 Standalone apps are Universal Binary with valid signatures; plugin code `WzMd` is in the generated build
- [x] 7.2 Run `openspec validate --all`; verify it passes
- [x] 7.3 Launch `WvZz Modulation.app`; verify the window opens, all cards and the three center tabs render, and the LFO phase scope markers move

## Workflow follow-up

- Archive this change before the PR merges.
- PR title: `feat(modulation): add WvZz Modulation plugin (chorus, phaser, flanger, tremolo)`, with `Closes #15`.
- The DoD item "checked by hand in the Standalone app" needs a person to turn the knobs and listen.
