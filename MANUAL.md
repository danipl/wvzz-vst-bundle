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
| `make vst3` | Builds the `.vst3` bundle into `build/WvZzDistortion_artefacts/VST3/WvZz Distortion.vst3` |
| `make standalone` | Builds the Standalone application into `build/WvZzDistortion_artefacts/Standalone/` |
| `make install-vst3` | Builds and copies the `.vst3` bundle directly to `~/Library/Audio/Plug-Ins/VST3/` |
| `make install-system` | Copies `.vst3` bundle to `/Library/Audio/Plug-Ins/VST3/` (requires `sudo`) |
| `make test` | Runs the zero-allocation DSP quantitative tests (`wvzz_dsp_tests`) |
| `make clean` | Cleans the build folder |

---

## 3. macOS Installation Guide

### Option A: Install for Current User (`~/Library/Audio/Plug-Ins/VST3`)
```bash
make install-vst3
```
This command compiles the Universal Binary VST3, copies it to `~/Library/Audio/Plug-Ins/VST3/`, and re-signs it with an ad-hoc signature.

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
