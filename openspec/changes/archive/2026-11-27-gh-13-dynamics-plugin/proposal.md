# Proposal

## Why

WvZz Dynamics is coded in commit `728b5d2`, but that commit is only on the branch `feat/luminous-dark-glass-ui-redesign`. That branch also holds `29964cf`, which `main` already has in squashed form as `02077e9`, so the branch cannot be merged as it is. The plugin also has no `README.md` and no `USER_MANUAL.md`, and the root README lists it as "Planned". Ticket: GH-13 (part of epic GH-12).

## What Changes

- Cherry-pick `728b5d2` onto a branch cut from fresh `main`. The trees of `02077e9` and `29964cf` are identical, so no conflict is expected.
- Add `Plugins/WvZzDynamics/README.md` and `Plugins/WvZzDynamics/USER_MANUAL.md`, with the same structure as the Distortion docs. Parameter names, ranges, and the factory presets come from the module code.
- Update the root `README.md`: the WvZz Dynamics roadmap row shows it as available, and a plugin section describes it. Update the same roadmap row in `Plugins/WvZzDistortion/README.md`.
- Correct the `dynamics-dsp-engine` spec where it differs from the code (ticket scope: "the spec matches the code").

### Out of Scope

- New Dynamics features or DSP changes.
- Build file refactoring (GH-14).

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `dynamics-dsp-engine`: two requirements name parameter ids and ranges that differ from the code. The spec is corrected to the code, with no DSP change: `trans_attack` / `trans_sustain` are `-1.0 to +1.0` (not dB), `trans_gate` is `-80 to 0 dB` (not `-90 to -12 dB`), and the sheen parameter id is `ott_sheen` (not `ott_high_sheen`).

The other Dynamics requirements arrive unchanged with commit `728b5d2` (archived change `2026-11-07-add-wvzz-dynamics-plugin`).

## Impact

- Code brought to `main`: `Plugins/WvZzDynamics/`, 5 modules under `Source/Modules/` (`DynDetector`, `TransientShaper`, `MultibandOTT`, `RhythmicPumper`, `MaximizerClipper`), `CMakeLists.txt`, `Makefile`, `Tests/DSPTests.cpp`, living specs.
- New docs: `Plugins/WvZzDynamics/README.md`, `Plugins/WvZzDynamics/USER_MANUAL.md`.
- Edited docs: root `README.md` (not the version badge).
- Release: the PR title is `feat(dynamics): ...`, so release-please proposes a minor version bump.
