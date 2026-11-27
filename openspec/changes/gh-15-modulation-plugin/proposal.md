# Proposal

## Why

The README roadmap promises WvZz Modulation: chorus, phaser, flanger, and tremolo. The repository has only a small `Flanger` module with 3 parameters and fixed feedback, center delay, and depth. No Chorus, Phaser, or Tremolo module exists. Ticket: GH-15 (part of epic GH-12).

## What Changes

- New modules in `Source/Modules/`: `Chorus` (prefix `chor`), `Phaser` (prefix `phs`), `Tremolo` (prefix `trem`). Each has a DSP class, a module class, and a view class.
- `Flanger` gets four new parameters: `flg_feedback`, `flg_depth`, `flg_center`, `flg_tzero`. Their defaults equal today's fixed values, so WvZz Distortion sounds the same. The Distortion UI does not show them.
- New plugin `Plugins/WvZzModulation/` (plugin code `WzMd`): fixed chain Chorus → Phaser → Flanger → Tremolo → Master, with the shared LFO / X/Y modulation module and 10 factory presets.
- The center column gets an optional third tab for a plugin-specific display. WvZz Modulation uses it for an LFO phase scope that shows the LFO position of all four effects.
- A third modulation profile (`Modulation`) with 14 targets for the new plugin.
- Docs: `Plugins/WvZzModulation/README.md`, `USER_MANUAL.md`, root `README.md` roadmap row, help topics for the new modules.
- Tests in `Tests/DSPTests.cpp`: zero allocation per new module, one test per spec scenario, Flanger default equivalence.

### Out of Scope

- New Flanger controls in the Distortion UI.
- Free reordering of the four effects.
- Any Delay or Reverb work.
- Tempo sync for the four effect LFOs (the shared LFO 1 / LFO 2 already sync to the host).

## Capabilities

### New Capabilities

- `modulation-fx-dsp-engine`: the Chorus, Phaser, and Tremolo effects and the WvZz Modulation serial chain, presets, and bypass behavior.

### Modified Capabilities

- `dsp-engine`: the "Stereo Modulated Flanger" requirement gets adjustable feedback, depth, center delay, and a through-zero mode.
- `plugin-ui-system`: a plugin can add one custom display tab to the center column; LFO phase scope.
- `plugin-host-integration`: WvZz Modulation build identity and tail length reporting.
- `modulation-system`: modulation target profile for WvZz Modulation.

## Impact

- New: `Source/Modules/{Chorus,Phaser,Tremolo}/`, `Source/Core/DSP/LfoPhaseTap.h`, `Source/Core/UI/LfoPhaseScopeComponent.{h,cpp}`, `Source/Core/UI/ModuleViewControls.h`, `Plugins/WvZzModulation/`.
- Changed: `Source/Modules/Flanger/`, `Source/Modules/Modulation/` (profile, custom tab), `Source/Core/Interfaces/ProcessContext.h` (new `ModTarget` values, appended), `Source/Core/UI/HelpContentRepository.h`, `CMakeLists.txt`, `Makefile`, `Tests/DSPTests.cpp`, `README.md`, `MANUAL.md`.
- Saved sessions of WvZz Distortion and WvZz Dynamics stay valid: no parameter id or choice index changes; new `ModTarget` values are appended.
- Release: PR title `feat(modulation): ...` gives a minor version bump.
