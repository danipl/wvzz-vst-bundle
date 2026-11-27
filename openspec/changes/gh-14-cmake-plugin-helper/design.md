# Design

## Context

The Distortion and Dynamics blocks in `CMakeLists.txt` differ only in target name, `PLUGIN_CODE`, and `PRODUCT_NAME`. Both list the same 5 source files under `Plugins/<Target>/`.

## Goals / Non-Goals

**Goals:**
- One place for plugin build settings. A new plugin costs one CMake line and one Makefile list entry.
- Identical build products before and after.

**Non-Goals:**
- Globbing module sources or changing the test target.

## Decisions

1. **Function, not macro.** `function(wvzz_add_plugin TARGET CODE PRODUCT_NAME)` keeps its variables local.
2. **Explicit source list inside the function** (`PluginProcessor.h/.cpp`, `PluginEditor.h/.cpp`, `Presets/FactoryPresets.h` under `Plugins/${TARGET}/`), not `file(GLOB)`. Every plugin follows this layout, and CMake re-runs correctly without glob checks. `ARGN` is passed to `target_sources` so a plugin can add extra files later.
3. **Makefile plugin list as `key:Target:Product Name` records.** GNU make lists split on spaces, and product names contain a space. So the list stores CMake target names (`PLUGIN_TARGETS := WvZzDistortion WvZzDynamics`) and a per-target product name variable (`PRODUCT_WvZzDistortion := WvZz Distortion`). Loops run in the shell over the target list. The short per-plugin targets (`distortion-vst3`) come from a `define` template and a lowercase key variable (`KEY_WvZzDistortion := distortion`).
4. **Install uses one shell helper** that copies and ad-hoc signs one bundle; `install` and `install-system` call it per plugin (with `sudo` for the system path).

## File Changes

| File | Change |
| :--- | :--- |
| `CMakeLists.txt` | `wvzz_add_plugin()` + 2 calls replace 2 blocks (about 120 lines removed) |
| `Makefile` | Plugin list, generated per-plugin targets, loops in `vst3`, `standalone`, `install*`, `status`, `help` |
| `MANUAL.md` | Build target table: all plugins, how to add one |

## Risks / Trade-offs

- [A setting is lost in the refactor] → Compare bundle list, `lipo -archs`, `codesign --verify`, and the plugin code in each `Info.plist` / `moduleinfo.json` before and after, from a clean build.
