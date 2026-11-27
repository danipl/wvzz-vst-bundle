# Proposal

## Why

Work on epic GH-12 found four defects outside its tickets. This change fixes three of them (the Standalone crash has its own change):

1. `Tests/DSPTests.cpp` uses `assert()`, but the test binary is built in Release mode, where `assert()` is compiled out. The 19 older suites could not fail.
2. WvZz Dynamics presets set only the parameters they list, so values from the previous preset stayed. Its DICE button changed internal smoothers only when they were moving, so in practice it did nothing and the host never saw a change.
3. WvZz Distortion, WvZz Dynamics, and WvZz Modulation did not save the selected preset with their state. After a reload the preset box showed preset 1.

Turning the old asserts into real checks then showed three failures: two test setup defects and one real DSP bug in the Master sub-bass mono maker.

## What Changes

- Tests: `assert()` in `Tests/DSPTests.cpp` becomes a check that always runs, prints the failed condition, and exits with code 1.
- Tests: `testBandRejectRecombination` sets the filter band it needs. The Magic EQ pre-distortion check runs on a plugin, where the switch exists as a parameter.
- **Master sub-bass mono maker (bug fix):** it now high-passes the stereo difference (side) signal at 120 Hz and leaves the mid signal untouched. Before, out-of-phase 60 Hz kept about 73% of its level; now about 6%. Mono content is unchanged. This affects every plugin when `master_mono_bass` is on.
- WvZz Dynamics: a preset first resets every parameter to its default. DICE sets 20 host parameters inside safe bounds.
- All plugins save the selected preset index with their state.
- New test suites: `testMasterMonoBass`, `testPresetStateAndDice`. Manuals updated.

### Out of Scope

- The Standalone crash inside JUCE (separate change).
- DICE in WvZz Distortion (it already writes host parameters).

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `dsp-engine`: how the sub-bass mono maker treats mono and out-of-phase lows.
- `dynamics-dsp-engine`: preset loading and DICE in WvZz Dynamics.
- `plugin-host-integration`: selected preset saved with the state; test checks that run in the Release build.

## Impact

- `Source/Modules/Master/MasterDSP.{h,cpp}`, `Plugins/WvZzDynamics/PluginProcessor.cpp`, `Plugins/WvZzDistortion/PluginProcessor.cpp`, `Plugins/WvZzModulation/PluginProcessor.cpp`, `Tests/DSPTests.cpp`, four user manuals.
- Sound: with `master_mono_bass` on, wide bass is now really reduced to mono. Sessions that used the switch will sound tighter in the lows.
- Sessions saved before this change load as before and show preset 1, as they did.
- Release: PR title `fix: ...` gives a patch version bump.
