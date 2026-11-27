# Proposal

## Why

The README roadmap promises WvZz Reverb: plate, hall, and shimmer spaces. The repository has no reverb code. It is the fifth and last plugin of the roadmap. Ticket: GH-17 (part of epic GH-12).

## What Changes

- New `EarlyReflections` module (prefix `er`): a multi-tap pattern with all-pass diffusion. It gives the first echoes of a room and feeds the main reverb.
- New `Reverb` module (prefix `rev`): one Feedback Delay Network (FDN: 8 delay lines that feed each other through a mixing matrix) with three modes: Plate, Hall, Shimmer. Controls: pre-delay, size, decay, low damping, high damping, diffusion, tail modulation, width, shimmer amount, shimmer interval, mix, bypass with tail fade-out.
- Shimmer shifts the tail up in pitch inside the feedback loop with the `dsp::GrainEngine` from WvZz Delay. The delay lines are the shared `dsp::DelayLine`.
- New plugin `Plugins/WvZzReverb/` (plugin code `WzRv`) with the shared modulation and master modules, 10 factory presets, and a decay-curve display in the custom center tab.
- A `Reverb` modulation profile with 14 targets.
- Docs: plugin `README.md`, `USER_MANUAL.md`, help topics. The root README shows all five plugins as available.
- Tests for every spec scenario in `Tests/DSPTests.cpp`.

### Out of Scope

- Gated or reverse reverb, convolution and impulse response loading, a freeze function.

## Capabilities

### New Capabilities

- `reverb-dsp-engine`: the early reflections, the 8-line FDN with its three modes, and the WvZz Reverb chain and presets.

### Modified Capabilities

- `plugin-ui-system`: decay-curve display and WvZz Reverb editor layout.
- `plugin-host-integration`: WvZz Reverb build identity and its tail length.
- `modulation-system`: modulation target profile for WvZz Reverb.

## Impact

- New: `Source/Modules/EarlyReflections/`, `Source/Modules/Reverb/`, `Source/Core/UI/ReverbDecayDisplayComponent.{h,cpp}`, `Plugins/WvZzReverb/`.
- Changed: `Source/Core/Interfaces/ProcessContext.h` (appended `ModTarget` values), `Source/Modules/Modulation/ModulationDSP.h`, `Source/Core/UI/HelpContentRepository.h`, `CMakeLists.txt`, `Makefile`, `Tests/DSPTests.cpp`, `README.md`, `MANUAL.md`, `Plugins/WvZzDistortion/README.md`.
- Existing plugins: no parameter id, choice index, or `ModTarget` value changes. `DelayLine` and `GrainEngine` are used as they are.
- Release: PR title `feat(reverb): ...` gives a minor version bump.
