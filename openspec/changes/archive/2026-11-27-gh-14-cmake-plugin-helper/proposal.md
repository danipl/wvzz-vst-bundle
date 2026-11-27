# Proposal

## Why

`CMakeLists.txt` repeats a block of about 60 lines for each plugin, and the `Makefile` names each plugin by hand in its build, install, and status targets. Three more plugins are planned (epic GH-12), which would mean five copies. Ticket: GH-14.

## What Changes

- Add a CMake function `wvzz_add_plugin(<Target> <PluginCode> <ProductName>)` that holds everything the two plugin blocks share.
- Replace the Distortion and Dynamics blocks with two calls.
- In the `Makefile`, define one plugin list. `vst3`, `standalone`, `install`, `install-vst3`, `install-system`, and `status` loop over it. The per-plugin targets (`distortion-vst3`, `dynamics-standalone`, ...) stay and are generated from the list.
- Update the build target table in `MANUAL.md`.

### Out of Scope

- New plugins, CI changes, any DSP or UI change.
- The `wvzz_dsp_tests` target and the `WvZzModules` source list.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

None. Build output is identical (same bundle names, plugin codes `WzDs` / `WzDy`, formats, architectures). `.openspec.yaml` sets `skip_specs: true`.

## Impact

- `CMakeLists.txt`, `Makefile`, `MANUAL.md`.
- No release: the PR title is `refactor(build): ...`.
