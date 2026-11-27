# Design

## Context

See proposal.md - Why. `git diff 02077e9 29964cf` is empty, so `728b5d2` applies on `main` exactly as it applied on its original parent.

## Goals / Non-Goals

**Goals:**
- Dynamics code on `main` with unchanged behavior.
- Docs that match the code: every module, every parameter range, every factory preset.

**Non-Goals:**
- Any DSP, UI, or build refactoring.

## Decisions

1. **Cherry-pick, not rebase or merge.** One commit is missing from `main`. A cherry-pick brings only that commit. A merge of the old branch would bring `29964cf` again and conflict with the squash `02077e9`.
2. **No delta specs (`skip_specs: true`).** The living specs for Dynamics arrive with the cherry-pick. Docs do not change behavior.
3. **Docs follow the Distortion layout.** `README.md`: overview, signal flow diagram, module table, presets, build commands. `USER_MANUAL.md`: one section per module with a parameter table (name, id, range, default), then the factory presets. Values are read from each module's `createParameterLayout` code and from `Plugins/WvZzDynamics/Presets/FactoryPresets.h`.
4. **Root README.** Change the roadmap row status to released and correct its description to the modules that exist. Add a section after the Distortion section that links to the plugin docs.

## File Changes

| File | Change |
| :--- | :--- |
| (cherry-pick `728b5d2`) 68 files | Dynamics plugin, modules, tests, specs, build files |
| `Plugins/WvZzDynamics/README.md` | New |
| `Plugins/WvZzDynamics/USER_MANUAL.md` | New |
| `README.md` | Roadmap row + plugin section |

## Risks / Trade-offs

- [The cherry-picked code fails to build or test on current `main`] → Run `make build`, `make test`, `make package` right after the cherry-pick and fix only what blocks them, with no behavior change.
- [Docs drift from code] → Each parameter table is checked against the module source before the task is ticked.
