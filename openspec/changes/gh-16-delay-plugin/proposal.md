# Proposal

## Why

The README roadmap promises WvZz Delay: tape, analog BBD (bucket-brigade device, an old analog delay chip with a dark sound), granular delay, ducking, and filter diffusion. The repository has no general delay line and no grain engine. Ticket: GH-16 (part of epic GH-12). WvZz Reverb (GH-17) needs both parts.

## What Changes

- New reusable `dsp::DelayLine` (fractional read, 3rd-order interpolation) with no plugin-specific code.
- New `Delay` module (prefix `dly`): free or tempo-synced time up to 4 seconds, Stereo or Ping-Pong routing, four modes (Clean, Tape, BBD, Granular), feedback 0% to 110% with high-pass, low-pass, and a soft clipper in the loop, all-pass diffusion, ducking, and a bypass that lets the tail fade out.
- New `Grain` module (prefix `grain`): a grain engine with 32 fixed voices and pitch shift. The Delay module uses it for Granular mode. It also works alone as a module.
- New plugin `Plugins/WvZzDelay/` (plugin code `WzDl`) with the shared modulation and master modules, 10 factory presets, and a delay-tap display in the custom center tab.
- A `Delay` modulation profile with 13 targets.
- Docs: plugin `README.md`, `USER_MANUAL.md`, root README roadmap row and section, help topics.
- Tests for every spec scenario in `Tests/DSPTests.cpp`.

### Out of Scope

- Reverb algorithms. Multi-tap pattern editing. Refactoring the Flanger or Chorus to use the new delay line.

## Capabilities

### New Capabilities

- `delay-dsp-engine`: the delay line, the Delay module with its four modes, the grain engine, and the WvZz Delay chain and presets.

### Modified Capabilities

- `plugin-ui-system`: delay-tap display and WvZz Delay editor layout.
- `plugin-host-integration`: WvZz Delay build identity and its tail length.
- `modulation-system`: modulation target profile for WvZz Delay.

## Impact

- New: `Source/Modules/Delay/`, `Source/Modules/Grain/`, `Source/Core/UI/DelayTapDisplayComponent.{h,cpp}`, `Plugins/WvZzDelay/`.
- Changed: `Source/Core/Interfaces/ProcessContext.h` (appended `ModTarget` values), `Source/Modules/Modulation/ModulationDSP.h` (profile), `Source/Core/UI/HelpContentRepository.h`, `CMakeLists.txt`, `Makefile`, `Tests/DSPTests.cpp`, `README.md`, `MANUAL.md`, `Plugins/WvZzDistortion/README.md`.
- Existing plugins: no parameter id, choice index, or `ModTarget` value changes.
- Memory: two 4-second delay lines per instance (about 2 x 262,144 floats at 48 kHz), allocated in prepare.
- Release: PR title `feat(delay): ...` gives a minor version bump.
