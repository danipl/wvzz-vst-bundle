# CamelPhat 3 User & Developer Manual

A practical guide to compiling, installing, and using CamelPhat 3 in your DAW.

---

## Table of Contents
1. [System Requirements](#1-system-requirements)
2. [Compilation Guide](#2-compilation-guide)
3. [macOS Installation](#3-macos-installation)
4. [Using CamelPhat in Ableton Live & Other DAWs](#4-using-camelphat-in-ableton-live--other-daws)
5. [Signal Flow & Architecture](#5-signal-flow--architecture)
6. [Module Reference & Plain-English Guide](#6-module-reference--plain-english-guide)
7. [Modulation & Factory Presets](#7-modulation--factory-presets)

---

## 1. System Requirements

- **Operating System:** macOS 11.0 (Big Sur) or newer.
- **Architectures:** Universal Binary (Apple Silicon `arm64` and Intel `x86_64`).
- **Plugin Formats:** VST3, Audio Unit (AU v2 `aumf`), and Standalone application.
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
| `make vst3` | Builds the `.vst3` bundle file into `build/CamelPhat_artefacts/VST3/CamelPhat.vst3` |
| `make install-vst3` | Builds and copies the `.vst3` bundle directly to `~/Library/Audio/Plug-Ins/VST3/` |
| `make install-system` | Copies `.vst3` bundle to `/Library/Audio/Plug-Ins/VST3/` (prompts for `sudo` if needed) |
| `make au` | Builds the Audio Unit (`.component`) bundle |
| `make install` | Builds both VST3 and AU and copies them to `~/Library/Audio/Plug-Ins/` |
| `make test` | Runs the quantitative DSP tests and allocation checks |
| `make validate-au` | Runs Apple's `auval` to test Audio Unit host integration |
| `make clean` | Cleans the build folder |

---

## 3. macOS Installation Guide

### Option A: Install for Current User (`~/Library/Audio/Plug-Ins/VST3`)
```bash
mkdir -p ~/Library/Audio/Plug-Ins/VST3
cp -R build/CamelPhat_artefacts/VST3/CamelPhat.vst3 ~/Library/Audio/Plug-Ins/VST3/
```
*(Or simply run `make install-vst3`)*.

### Installing Audio Unit (AU)
```bash
mkdir -p ~/Library/Audio/Plug-Ins/Components
cp -R build/CamelPhat_artefacts/AU/CamelPhat.component ~/Library/Audio/Plug-Ins/Components/
killall -9 AudioComponentRegistrar 2>/dev/null || true
```
*(Or simply run `make install`)*.

---

## 4. Signal Flow & Architecture

CamelPhat 3 processes stereo audio (`Left`, `Right`) through a strict serial/parallel topology with hardware-free bypass switches for every module:

```
[Input Gain] (-24 dB to +24 dB)
     │
     ▼
┌────────────────────────────────────────────────────────┐
│ 1. BP Filter (Bandpass Isolation & Band-Reject Split)  │
└───────────────┬────────────────────────────────┬───────┘
                │ (Passband Signal)              │ (Rejected Band Signal:
                ▼                                │  Low < LowCut + High > HighCut)
┌───────────────────────────────┐                │
│ 2. Distortion (4x Parallel)   │                │
│  [Mech] [Tube] [Bit] [Xcita]  │                │
└───────────────┬───────────────┘                │
                │                                │
                ▼                                │
┌───────────────────────────────┐                │
│ 3. MM Filter (Multi-Mode)     │                │
│    + Envelope Follower        │                │
└───────────────┬───────────────┘                │
                │                                │
                ◄────────────────────────────────┘
                │ (Summing Node: Passband + (Rejected Band * BR Mix))
                ▼
┌───────────────────────────────┐
│ 4. Flanger (Stereo Modulated) │
└───────────────┬───────────────┘
                ▼
┌───────────────────────────────┐
│ 5. Magic EQ (Resonant Bass)   │
└───────────────┬───────────────┘
                ▼
┌───────────────────────────────┐
│ 6. Compressor (One-Knob)      │
└───────────────┬───────────────┘
                ▼
┌───────────────────────────────┐
│ 7. Master Mix & Output Volume │
└───────────────────────────────┘
```

---

## 5. Module Reference & Plain-English Guide

### In-Plugin Help System
Click the circular **`?`** button in the header of any module to open an instant explanation bubble. It describes the module's audio role, signal placement, and parameter functions.

### Module 1: BP Filter (Bandpass + Band-Reject Split)
- **Low Cut & High Cut (20 Hz - 20 kHz):** Andy Simper ZDF State Variable Filter crossover frequencies.
- **Link:** Locks logarithmic frequency delta between Low Cut and High Cut.
- **Low Res & High Res (0.0 to 1.0):** Q factors mapped from `0.707` to `12.0`.
- **BR Mix (0% to 100%):** Recombines the clean rejected low/high frequencies directly post-MM Filter.

### Module 2: Distortion (4 Parallel Circuits + 4x Oversampling)
- **Mech (Drive 1.0 to 25.0):** 1.2 kHz pre-emphasis with asymmetric sine wavefolding and negative clipping.
- **Tube (Drive 1.0 to 15.0):** Warm analog-style saturation with DC blocking.
- **Bitcrusher (0.0 to 1.0):** Bit-depth reduction (16 down to 4 bit) and sample-and-hold decimation (1x to 32x) evaluated at the base sample rate.
- **Xcita (0.0 to 1.0):** 2.8 kHz 2-pole Butterworth HPF with full-wave rectification.

### Module 3: MM Filter (12 Multi-Mode Topologies)
- **Modes:** `LP 12dB`, `LP 24dB` (cascaded SVF with internal `tanh` feedback saturation), `BP 12/24dB`, `HP 12/24dB`, `Notch`, `Peak`, `Comb +`, `Comb -`, `RingMod`, and `Vowel / Formant` morphing across `A-E-I-O-U`.
- **Envelope Follower:** Tapped directly from the pre-BP input buffer with `0.1-1000 ms` attack and `1-5000 ms` release, applying `±6 octaves` cutoff modulation.

### Module 4: Flanger
- **Stereo Delay Line:** 3rd-order Lagrange fractional interpolation (`2.5 ms ± 2.0 ms`).
- **Quadrature LFO:** 90° stereo phase offset between Left and Right channels with fixed `0.55` internal feedback.

### Module 5: Magic EQ
- **Resonant Low Boost:** Tuned to `meq_tune` (60 Hz to 120 Hz, default 80 Hz) with +12 dB boost.
- **Mud Dip:** Complimentary attenuation at `2 * meq_tune` (-2.5 dB).
- **Phat Mode:** Soft clipper inside the resonant feedback state and +3 dB air shelf at 8 kHz.

### Module 6: Compressor
- **One-Knob Operation:** `comp_amount` simultaneously drives threshold (0 to -36 dB) and auto-makeup (0 to +14 dB).
- **Normal Mode:** 3.5:1 ratio, 3.5 ms attack, 6 dB soft knee.
- **Phat Mode:** 12:1 ratio, 0.8 ms attack, 1 dB hard knee, and `-0.1 dBFS` `tanh` ceiling.

### Module 7: Master Section
- **Dry Latency Compensation:** Global dry path delay buffer matching the 4x distortion oversampler delay.
- **Dice:** Smart parameter randomizer ensuring musically useful presets without runaway resonance.

---

## 6. Modulation & Factory Presets

### Modulation Engine
- **LFO 1 & LFO 2:** Control-rate (every 32 samples) modulation with per-sample linear interpolation.
- **7 Waveforms:** Sine, Triangle, Saw Down, Saw Up, Square, Sample & Hold, Smooth Random.
- **Tempo Sync:** Free (0.02 Hz - 30.0 Hz) and Host Sync (4/1 down to 1/32, including Dotted and Triplet).
- **21 Targets Matrix:** Including cross-modulation of LFO 2 Rate by LFO 1.
- **Macro X/Y Pad:** 2D interactive surface applying bipolar/unipolar offsets to assignable targets.

### 10 Factory Presets
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
