# WvZz Distortion - User & Developer Manual

A practical guide to compiling, installing, and using WvZz Distortion in your DAW.

---

## Table of Contents
1. [System Requirements](#1-system-requirements)
2. [Compilation & Fast Build Guide](#2-compilation--fast-build-guide)
3. [macOS Installation Guide](#3-macos-installation-guide)
4. [Signal Flow & Architecture](#4-signal-flow--architecture)
5. [SOLID Architecture Reference](#5-solid-architecture-reference)
6. [Module Reference & Plain-English Guide](#6-module-reference--plain-english-guide)
7. [Modulation & Factory Presets](#7-modulation--factory-presets)

---

## User Guides & Plugin Documentation

For dedicated plugin manuals and parameter references:
- **WvZz Distortion**:
  - [Plugin Architectural Description](Plugins/WvZzDistortion/README.md)
  - [Producer User Manual](Plugins/WvZzDistortion/USER_MANUAL.md)

---

## 1. System Requirements

- **Operating System:** macOS 11.0 (Big Sur) or newer.
- **Architectures:** Universal Binary (Apple Silicon `arm64` and Intel `x86_64`).
- **Plugin Formats:** VST3 and Standalone desktop application. (AU is not supported).
- **Real-Time Audio Guarantee:** Zero dynamic memory heap allocations on the audio processing thread; full denormal and NaN protection.
- **Build Tools:**
  - Xcode Command Line Tools (`clang` supporting C++20).
  - CMake 3.22+.
  - Ninja build system.

---

## 2. Compilation & Fast Build Guide

### Quick Build with `make` (Recommended)
A root [`Makefile`](Makefile) provides simple commands:

| Command | Action |
| :--- | :--- |
| `make build` | Builds everything: the `WvZzModules` library, every plugin (VST3 + Standalone), and the tests |
| `make vst3` | Builds the `.vst3` bundle of every plugin into `build/<Target>_artefacts/Release/VST3/` |
| `make standalone` | Builds the Standalone app of every plugin into `build/<Target>_artefacts/Release/Standalone/` |
| `make distortion-vst3` / `make distortion-standalone` | Builds only WvZz Distortion |
| `make dynamics-vst3` / `make dynamics-standalone` | Builds only WvZz Dynamics |
| `make modulation-vst3` / `make modulation-standalone` | Builds only WvZz Modulation |
| `make delay-vst3` / `make delay-standalone` | Builds only WvZz Delay |
| `make reverb-vst3` / `make reverb-standalone` | Builds only WvZz Reverb |
| `make install` (or `make install-vst3`) | Builds, copies, and ad-hoc signs every `.vst3` bundle in `~/Library/Audio/Plug-Ins/VST3/` |
| `make install-system` | Copies and signs every `.vst3` bundle in `/Library/Audio/Plug-Ins/VST3/` (requires `sudo`) |
| `make test` | Runs the zero-allocation DSP quantitative tests (`wvzz_dsp_tests`) |
| `make package` | Builds, tests, verifies Universal Binary + signature of every bundle, and writes the release zips to `dist/` |
| `make status` | Shows which plugins are installed |
| `make clean` | Removes the build folder |

`make help` prints this list from the Makefile.

### Adding a Plugin to the Build
The build reads one plugin list. A new plugin needs three steps:

1. Create `Plugins/<Target>/` with `PluginProcessor.h/.cpp`, `PluginEditor.h/.cpp`, and `Presets/FactoryPresets.h`.
2. In [`CMakeLists.txt`](CMakeLists.txt), add one line: `wvzz_add_plugin(<Target> <PluginCode> "<Product Name>")`.
3. In the [`Makefile`](Makefile), add `<Target>` to `PLUGIN_TARGETS` and define `KEY_<Target>` and `PRODUCT_<Target>`.

`make vst3`, `make standalone`, `make install`, `make status`, and `make <key>-vst3` then include the new plugin.

---

## 3. macOS Installation Guide

### Option A: Install for Current User (`~/Library/Audio/Plug-Ins/VST3`)
```bash
make install-vst3
```
This command compiles every plugin as a Universal Binary VST3, copies each bundle to `~/Library/Audio/Plug-Ins/VST3/`, and re-signs it with an ad-hoc signature.

---

## 4. Signal Flow & Architecture

WvZz Distortion processes stereo audio (`Left`, `Right`) through a serial/parallel topology with independent bypass switches for every module:

```
[Input Gain] (-24 dB to +24 dB)
     │
     ▼
┌────────────────────────────────────────────────────────┐
│ 1. BP Filter (Bandpass Isolation & Band-Reject Split)  │
└───────────────┬────────────────────────────────┬───────┘
                │ (Passband Signal)              │ (Rejected Band Signal:
                ▼                                │  Low < LowCut + High > HighCut)
┌──────────────────────────────────────────────┐ │
│ 2. Distortion (4 Parallel Circuits, 4x OVS)   │ │
└───────────────┬──────────────────────────────┘ │
                ▼                                │
┌──────────────────────────────────────────────┐ │
│ 3. MM Filter (12 Topologies + Envelope Follow)│ │
└───────────────┬──────────────────────────────┘ │
                │                                │
                ▼                                │
          [Summing Node] ◄───────────────────────┘
          (Passband + Rejected Band * BR Mix)
                │
                ▼
┌──────────────────────────────────────────────┐
│ 4. Stereo Flanger (Quadrature LFO)           │
└───────────────┬──────────────────────────────┘
                ▼
┌──────────────────────────────────────────────┐
│ 5. Magic EQ (Sub Peaking Boost & Notch)      │
└───────────────┬──────────────────────────────┘
                ▼
┌──────────────────────────────────────────────┐
│ 6. Compressor (One-Knob Dynamic Leveling)    │
└───────────────┬──────────────────────────────┘
                ▼
┌──────────────────────────────────────────────┐
│ 7. Master Section (Latency Comp + Dry/Wet +  │
│    Volume Trim + Hard Clip + Telemetry)      │
└───────────────┬──────────────────────────────┘
                ▼
         [Stereo Output]
```

---

## 5. SOLID Architecture Reference

The codebase is built on SOLID software design principles:

- **Single Responsibility Principle (SRP):** DSP algorithms are decoupled from UI components and parameter storage. Each module directory under `Source/Modules/<Name>/` contains distinct `DSP`, `View`, and `Module` classes.
- **Open/Closed Principle (OCP):** New audio modules can be created and plugged into the bundle without altering host routing or existing modules.
- **Liskov Substitution Principle (LSP):** All modules implement the `IPluginModule` interface and can be managed polymorphically.
- **Interface Segregation Principle (ISP):** Clear separation between audio rendering (`IDSPModule`), visual metering (`IMeterSource`), and UI presentation (`IModuleView`).
- **Dependency Inversion Principle (DIP):** Modules depend on the abstract `ProcessContext` data structure rather than on specific host implementations.

---

## 6. Module Reference & Plain-English Guide

- **BP Filter:** Separates audio into a passband and a rejected band. The rejected band can be recombined after distortion to preserve clean sub-bass or top-end sparkle.
- **Distortion:** Four parallel saturation circuits (Mechanical wavefolding, Tube warmth, Bitcrusher decimation, and High-frequency exciter) running through 4x oversampling.
- **MM Filter:** Multi-mode filter with 12 topologies, including low-pass, high-pass, band-pass, comb filters, ring modulation, and vowel formant filtering.
- **Flanger:** Stereo flanger with 90° quadrature phase offset for wide spatial movement.
- **Magic EQ:** Sub-bass peaking booster paired with an anti-mud cut at the second harmonic.
- **Compressor:** One-knob leveling amplifier with Normal (3.5:1) and Phat (12:1) modes.
- **Modulation:** Dual LFOs with 7 waveforms and tempo sync, plus an interactive 2D X/Y macro pad.
- **Master Section:** Handles latency compensation for distortion oversampling, master wet/dry blend, output volume trim, and stereo peak/RMS metering.

---

## 7. Modulation & Factory Presets

10 factory presets are included:
1. `Neuro Reese Mangler`
2. `Phat Breakbeat Smash`
3. `Sub Weight Enhancer`
4. `Acid Band-Reject Sweep`
5. `Lo-Fi Bit Crunch`
6. `Warm Tube Bus Glue`
7. `Stepped Comb Resonator`
8. `Talking Vowel Wobble`
9. `Metallic RingMod Perc`
10. `Wide Quadrature Flanger`

The plugin also includes a **Dice** button to intelligently randomize parameters within musical limits.
