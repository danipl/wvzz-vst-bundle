# Tasks

## 1. Test checks

- [x] 1.1 Redefine `assert` in `Tests/DSPTests.cpp` so it runs in Release; verify the test run now stops at the first false condition (it stopped at `Tests/DSPTests.cpp:335`)
- [x] 1.2 Fix the setup of `testBandRejectRecombination` and of the Magic EQ pre-distortion check; verify both pass

## 2. Master sub-bass mono maker

- [x] 2.1 Rework the mono maker to mid/side with a 24 dB/oct side high-pass; add `testMasterMonoBass` for the 4 scenarios; verify it passes (out-of-phase 60 Hz: RMS 0.021 of 0.354)

## 3. Presets, DICE, and state

- [x] 3.1 WvZz Dynamics: reset parameters before a preset and make DICE set host parameters; verify with `testPresetStateAndDice` (20 presets independent of history; DICE changed parameters in 20 of 20 runs)
- [x] 3.2 Save the selected preset index in WvZz Distortion, WvZz Dynamics, and WvZz Modulation; verify with `testPresetStateAndDice` for all 5 plugins
- [x] 3.3 Update the Dynamics manual (presets, DICE) and the mono bass line in four manuals; verify the old statements are gone

## 4. Integration check

- [x] 4.1 Run `make package`; verify 42 of 42 suites pass and 5 VST3 + 5 Standalone are Universal Binary with valid signatures
- [x] 4.2 Run `openspec validate --all`; verify it passes

## Workflow follow-up

- Archive this change before the PR merges.
- PR title: `fix: enforce test checks, mono bass, Dynamics presets and DICE, saved preset index`.
