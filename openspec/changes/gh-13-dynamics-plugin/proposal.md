# Proposal

## Why

WvZz Dynamics is coded in commit `728b5d2`, but that commit is only on the branch `feat/luminous-dark-glass-ui-redesign`. That branch also holds `29964cf`, which `main` already has in squashed form as `02077e9`, so the branch cannot be merged as it is. The plugin also has no `README.md` and no `USER_MANUAL.md`, and the root README lists it as "Planned". Ticket: GH-13 (part of epic GH-12).

## What Changes

- Cherry-pick `728b5d2` onto a branch cut from fresh `main`. The trees of `02077e9` and `29964cf` are identical, so no conflict is expected.
- Add `Plugins/WvZzDynamics/README.md` and `Plugins/WvZzDynamics/USER_MANUAL.md`, with the same structure as the Distortion docs. Parameter names, ranges, and the factory presets come from the module code.
- Update the root `README.md`: the WvZz Dynamics roadmap row shows it as available, and a plugin section describes it.

### Out of Scope

- New Dynamics features or DSP changes.
- Build file refactoring (GH-14).

## Capabilities

### New Capabilities

None.

### Modified Capabilities

None. The Dynamics requirements are already written: commit `728b5d2` carries the archived change `2026-11-07-add-wvzz-dynamics-plugin` and its synced living specs (`dynamics-dsp-engine` and 7 others). This change adds only docs, so `.openspec.yaml` sets `skip_specs: true`.

## Impact

- Code brought to `main`: `Plugins/WvZzDynamics/`, 5 modules under `Source/Modules/` (`DynDetector`, `TransientShaper`, `MultibandOTT`, `RhythmicPumper`, `MaximizerClipper`), `CMakeLists.txt`, `Makefile`, `Tests/DSPTests.cpp`, living specs.
- New docs: `Plugins/WvZzDynamics/README.md`, `Plugins/WvZzDynamics/USER_MANUAL.md`.
- Edited docs: root `README.md` (not the version badge).
- Release: the PR title is `feat(dynamics): ...`, so release-please proposes a minor version bump.
