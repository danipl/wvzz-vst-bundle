# Proposal

## Why

The Standalone apps of the bundle crashed now and then with a heap corruption signal: WvZz Dynamics, WvZz Modulation, and WvZz Delay, at launch or after some seconds (5 crash reports on 2026-11-27 between 07:46 and 08:09). An AddressSanitizer build located the cause inside JUCE 8.0.4, not in plugin code: `juce_CoreAudio_mac.cpp:795`, a write past the end of JUCE's own temporary audio buffer in the CoreAudio device callback. It happens when macOS delivers more samples per callback than JUCE prepared for, for example when another audio application changes the device buffer size.

JUCE fixed this upstream on 2025-06-03 in commit `f6df3e3c` ("CoreAudio: Respect buffer size passed to audio callback"). The fix is in JUCE 8.0.9 and later.

## What Changes

- Update the JUCE dependency from 8.0.4 to 8.0.15, the newest regular release of the 8.0 line.
- Replace three calls to `Font::getStringWidth` / `getStringWidthFloat`, which 8.0.15 removed, with `GlyphArrangement::getStringWidthInt` / `getStringWidth`. The logo keeps whole-pixel widths, so the layout does not move.
- In one test, open the editor through `createEditorIfNeeded()`; `createEditor()` is no longer public on the base class.
- Update the JUCE cache key in `.github/workflows/pr.yml`.

### Out of Scope

- JUCE 9. Any change to plugin DSP or parameters.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

None. This is a dependency update that removes a crash in the Standalone wrapper. No requirement changes, so `.openspec.yaml` sets `skip_specs: true`.

## Impact

- `CMakeLists.txt` (`GIT_TAG`, not the `VERSION` line), `.github/workflows/pr.yml`, `Source/Core/UI/WvZzLogoComponent.h`, `Source/Core/UI/HelpOverlayComponent.h`, `Tests/DSPTests.cpp`.
- All five plugins are rebuilt against a newer JUCE (VST3 and Standalone). 798 upstream commits lie between 8.0.4 and 8.0.12 alone, so a listening check in a host is advisable before a release.
- Release: PR title `fix(standalone): ...` gives a patch version bump.
