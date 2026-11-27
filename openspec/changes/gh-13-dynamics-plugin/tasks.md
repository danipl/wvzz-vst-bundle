# Tasks

## 1. Bring the Dynamics code to the branch

- [ ] 1.1 Cherry-pick `728b5d2` onto `feat/gh-13-dynamics-plugin`; verify `git status` is clean and `Plugins/WvZzDynamics/` exists
- [ ] 1.2 Run `make build`, `make test`, and `make package`; verify all pass and `make package` checks 2 VST3 bundles and 2 Standalone apps as arm64 + x86_64 with valid signatures

## 2. Plugin docs

- [ ] 2.1 Write `Plugins/WvZzDynamics/README.md` (overview, signal flow, modules, presets, build); verify every module in `Plugins/WvZzDynamics/PluginProcessor.cpp` is described
- [ ] 2.2 Write `Plugins/WvZzDynamics/USER_MANUAL.md`; verify each parameter id, range, and default matches the module code and every preset in `FactoryPresets.h` is listed

## 3. Root README

- [ ] 3.1 Update the WvZz Dynamics roadmap row and add a plugin section; verify `grep -n "Dynamics" README.md` shows no "Planned" and the version badge line is unchanged in `git diff`

## 4. Integration check

- [ ] 4.1 Run `openspec validate --all`; verify it passes
- [ ] 4.2 Check the new docs for the forbidden words in `AGENTS.md` section 8; verify `grep` finds none

## Workflow follow-up

- Archive this change before the PR merges (CI rejects active changes).
- Open the PR with the title `feat(dynamics): add WvZz Dynamics plugin with docs` and `Closes #13`.
