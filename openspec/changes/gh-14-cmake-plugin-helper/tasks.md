# Tasks

## 1. Baseline

- [ ] 1.1 Record the current bundles, `lipo -archs`, and plugin codes from the existing build; verify 2 VST3 + 2 Standalone are listed in the baseline file

## 2. CMake helper

- [ ] 2.1 Add `wvzz_add_plugin()` to `CMakeLists.txt` and replace both plugin blocks with calls; verify `grep -c juce_add_plugin CMakeLists.txt` is 1 and the `project(... VERSION ...)` line is unchanged in `git diff`

## 3. Makefile plugin list

- [ ] 3.1 Add the plugin list and rewrite `vst3`, `standalone`, per-plugin targets, `install`, `install-vst3`, `install-system`, `status`, and `help`; verify `make -n install` shows a copy and a codesign for both plugins and `make -n distortion-vst3 dynamics-standalone` resolves
- [ ] 3.2 Update the build target table in `MANUAL.md`; verify every documented target exists in `make help`

## 4. Integration check

- [ ] 4.1 Run `make clean`, then `make package`; verify build and tests pass and the bundle list, architectures, signatures, and plugin codes equal the baseline
- [ ] 4.2 Run `make install`; verify both `.vst3` bundles are in `~/Library/Audio/Plug-Ins/VST3/` and pass `codesign --verify --deep --strict`
- [ ] 4.3 Run `openspec validate --all`; verify it passes

## Workflow follow-up

- Archive this change before the PR merges.
- PR title: `refactor(build): add wvzz_add_plugin() helper and Makefile plugin list`, with `Closes #14`.
