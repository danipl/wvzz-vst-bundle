# WvZz Dynamics

> **Multiband and Rhythmic Dynamics Processor of the WvZz Audio Plugin Bundle**  
> Formats: **VST3** & **Standalone** for macOS Universal Binary (`arm64` & `x86_64`). No AU build. Built with JUCE 8 and C++20.

---

## 1. What is WvZz Dynamics?

**WvZz Dynamics** is a dynamics chain for electronic music. It shapes transients, compresses in three bands, glues a bus, ducks in time with the song, and clips the peaks.

It is one plugin with six processing modules in a fixed order, plus a modulation section and a master section.

| # | Module | Card title in the UI | Parameter prefix | Purpose |
| :--- | :--- | :--- | :--- | :--- |
| 1 | Sub & Detector | `SUB & DETECTOR` | `det_` | Splits the sub-bass away from the chain and filters the detector signal. |
| 2 | Transient Shaper | `TRANSIENT & GATE` | `trans_` | Boosts or cuts attack and sustain, and gates quiet noise. |
| 3 | Multiband OTT | `HYPER-OTT (3-BAND)` | `ott_` | Upward and downward compression in three bands, with a high-band boost. |
| 4 | Compressor | `COMPRESSOR` | `comp_` | Bus compressor with Opto, VCA, and FET modes. |
| 5 | Rhythmic Pumper | `RHYTHMIC PUMPER` | `pump_` | Ducks the level in time with the host tempo or an audio trigger. |
| 6 | Maximizer Clipper | `MAXIMIZER CLIPPER` | `clip_` | Adds drive and clips peaks at a set ceiling. |
| - | Modulation | `MODULATION & X/Y` | `lfo1_`, `lfo2_`, `xy_` | Two LFOs and an X/Y pad that move 15 targets. |
| - | Master | `MASTER` | `input_gain`, `master_` | Input gain, dry/wet mix, volume, mono bass, output clip, bypass. |

The plugin exposes 64 parameters. The [User Manual](USER_MANUAL.md) lists each one with its ID, range, and default.

---

## 2. Why WvZz Dynamics?

### The Problem
1. **Sub-bass drives everything.** A loud kick or sub makes a full-band compressor pump the mids and highs.
2. **Upward compression lifts noise.** OTT-style processing raises hiss and room tails between notes.
3. **Loudness needs several plugins.** A typical chain uses a transient shaper, an OTT, a compressor, a ducker, and a clipper.

### How WvZz Dynamics Handles It
- **Sub Lock.** A 24 dB/oct Linkwitz-Riley crossover removes the sub from the chain. The sub is added back after the Compressor.
- **Filtered detector.** The Compressor listens to a high-passed copy of the input, or to an external sidechain.
- **Gate before OTT.** The gate sits in the Transient Shaper, so the OTT does not lift the noise floor.
- **One chain.** All stages run in one plugin in a fixed order.
- **Anti-aliased clipping.** The Maximizer Clipper uses first-order ADAA (antiderivative anti-aliasing) to reduce aliasing without oversampling.
- **No added latency.** The plugin does not report latency to the host. The tail length is zero.

---

## 3. How It Works (Architecture & DSP)

### Signal Flow

```text
                  ┌──────────────────────────────┐
  Main In ───────►│ Input Gain (-24 to +24 dB)   │
                  └───────────────┬──────────────┘
                                  │
            ┌─────────────────────┼─────────────────────────┐
            │                     │                         │
            ▼ (main path)         ▼ (detector copy)         ▼ (dry copy)
  ┌───────────────────┐   ┌──────────────────────┐          │
  │ 1. Sub & Detector │   │ Detector source:     │          │
  │    LR4 crossover  │   │ Sidechain In bus if  │          │
  └───┬───────────┬───┘   │ active, else main in │          │
      │           │       └──────────┬───────────┘          │
      │ (above    │ (sub, when       ▼                      │
      │  X-Over)  │  LOCK is on)  ┌──────────────────────┐  │
      │           │               │ SC HPF + Tilt filter │  │
      ▼           │               └──────────┬───────────┘  │
  ┌───────────────────┐           (filtered detector)       │
  │ 2. Transient      │                      │              │
  │    Shaper + Gate  │                      │              │
  └─────────┬─────────┘                      │              │
            ▼     │                          │              │
  ┌───────────────────┐                      │              │
  │ 3. Multiband OTT  │                      │              │
  │  120 Hz / 2.5 kHz │                      │              │
  └─────────┬─────────┘                      │              │
            ▼     │                          │              │
  ┌───────────────────┐                      │              │
  │ 4. Compressor     │◄─────────────────────┤              │
  │  Opto / VCA / FET │  (level detection)   │              │
  └─────────┬─────────┘                      │              │
            ▼     │                          │              │
      [Sum] ◄─────┘ (sub added back)         │              │
            │                                │              │
            ▼                                │              │
  ┌───────────────────┐                      │              │
  │ 5. Rhythmic       │◄─────────────────────┘              │
  │    Pumper         │  (trigger, Audio / Ext SC mode)     │
  └─────────┬─────────┘                                     │
            ▼                                               │
  ┌───────────────────┐                                     │
  │ 6. Maximizer      │                                     │
  │    Clipper (ADAA) │                                     │
  └─────────┬─────────┘                                     │
            ▼                                               │
  [Master: Dry/Wet Mix] ◄───────────────────────────────────┘
            │
            ▼
  [Volume (-36 to +12 dB)]
            │
            ▼
  [Mono Bass below 120 Hz, if MONO is on]
            │
            ▼
  [Output Clip at 0 dBFS, if CLIP is on]
            │
            ▼
  [Master Bypass crossfade (10 ms)] ──► Stereo Out
```

The Modulation section runs first in every audio block. It computes the LFO and X/Y offsets that the other modules read.

### Module Breakdown

#### 1. Sub & Detector
- **Crossover**: 24 dB/oct Linkwitz-Riley split at `X-Over` (40 to 300 Hz).
- **Sub Lock**: when on, the sub band skips the OTT and the Compressor. The gate still applies to it.
- **Tight**: a level-dependent clamp on the sub band, with 5 ms attack and 60 ms release.
- **Detector filter**: a 12 dB/oct high-pass (`SC HPF`) and a tilt around 1 kHz (`Tilt`). It changes what the Compressor hears, not the audio.
- **Listen**: sends the filtered detector signal to the output so you can hear it.

#### 2. Transient Shaper
- **Method**: compares a fast and a slow envelope. The difference marks attack or sustain, at any input level.
- **Attack**: up to about +12 dB of boost on the attack.
- **Sustain**: lifts or cuts the tail.
- **Speed**: `Fast` (140 ms slow-envelope release) or `Punch` (250 ms).
- **Gate**: stereo-linked gate with 3 dB hysteresis and a hold time. It is off at -80 dB.

#### 3. Multiband OTT
- **Bands**: Low (below 120 Hz), Mid (120 Hz to 2.5 kHz), High (above 2.5 kHz). Crossovers are fixed.
- **Downward**: compresses band levels above -12 dBFS.
- **Upward**: lifts band levels between -55 dBFS and -22 dBFS.
- **Time**: scales the 5 ms attack and 50 ms release.
- **Sheen**: gain on the High band before its dynamics stage, 0 to +12 dB.
- **Depth**: blend between the phase-aligned dry signal and the processed bands.

#### 4. Compressor
- **Modes**: `Opto` (two-stage release), `VCA` (single-stage), `FET` (attack five times faster).
- **Controls**: threshold, ratio, attack, release, soft knee, and parallel mix.
- **Auto makeup**: makeup gain follows threshold and ratio. There is no manual makeup control.
- **PHAT**: forces at least 12:1 ratio, at most 0.8 ms attack, a 1 dB knee, and a soft clip at -0.1 dBFS.
- **Detector**: reads the filtered detector signal from module 1.

#### 5. Rhythmic Pumper
- **Modes**: `Tempo Sync` (locked to host beat position) or `Audio / Ext SC` (triggered when the detector passes -24 dBFS).
- **Divisions**: 9 values from `1/1` to `1/16`, plus dotted and triplet values.
- **Shapes**: `Classic Duck`, `Hard Kick Clear`, `Reverse Bass`, `Festival Swell`.
- **Sub Only**: ducks only the signal below 180 Hz.

#### 6. Maximizer Clipper
- **Drive**: 0 to +18 dB of gain into the clipper.
- **Ceiling**: -12 to 0 dB.
- **Modes**: `Hard Clip`, `Warm Tube` (tanh curve), `Hardstyle Crunch` (asymmetric curve).
- **Mix**: blends the clipped signal with the signal before the drive.

#### Modulation
- **Two LFOs**: 7 waveforms, free rate (0.02 to 30 Hz) or tempo sync (24 divisions), bipolar depth.
- **X/Y pad**: one target and one bipolar depth per axis.
- **15 targets**: Sub X-Over, SC HPF, Trans Attack, Trans Sustain, OTT Depth, OTT Upward, OTT Sheen, Comp Thresh, Comp Mix, Pump Amount, Pump Curve, Clip Drive, Master Mix, LFO 2 Rate, Master Vol.
- **Monitor view**: the same card can show an input/output comparison display instead of the X/Y pad.

#### Master
- **Input Gain**: applied before all modules and before the dry copy is taken.
- **Dry/Wet**: blends the dry copy with the processed signal.
- **MONO**: sums the signal below 120 Hz to mono.
- **CLIP**: hard clip at 0 dBFS. It is on by default.
- **Bypass**: 10 ms crossfade to the dry signal.

### Audio Buses
- **Input**: stereo.
- **Output**: stereo.
- **Sidechain**: optional stereo input, disabled by default. Enable it in the host to feed the detector from another track.

---

## 4. Factory Presets

The plugin ships with 20 factory presets:

1. Euphoric Lead Wall
2. Tok & Tail Kick Glue
3. Reverse Bass Pumper
4. Screech OTT Maximizer
5. Rawstyle Kick Clipper
6. Mainstage Drop Bus
7. Big Room Kick Snap
8. Festival Sub Lock
9. Pluck Transient Pop
10. SuperSaw Hyper-OTT
11. Clap & Snare Smash
12. Drum Bus VCA Glue
13. Hi-Hat Air & Sparkle
14. Tight Gate Percussion
15. Euphoric Vocal Up-Lift
16. Piano & Pluck Leveler
17. Sidechain Pad Swell
18. Club Loudness Maximizer
19. Parallel Bus Slam
20. Clean Mastering Glue

The [User Manual](USER_MANUAL.md#5-factory-presets) describes the use case of each preset.

---

## 5. Build & Install

Run these commands from the repository root. They need CMake and Ninja.

```bash
make dynamics-vst3         # Build only the WvZz Dynamics VST3
make dynamics-standalone   # Build only the WvZz Dynamics Standalone app
make vst3                  # Build all VST3 bundles in the repository
make test                  # Build and run the DSP tests
make install               # Copy all VST3 bundles to ~/Library/Audio/Plug-Ins/VST3/
make install-system        # Copy all VST3 bundles to /Library/Audio/Plug-Ins/VST3/ (needs sudo)
make status                # Show which WvZz plugins are installed
```

Build outputs:

| Format | Path |
| :--- | :--- |
| VST3 | `build/WvZzDynamics_artefacts/Release/VST3/WvZz Dynamics.vst3` |
| Standalone | `build/WvZzDynamics_artefacts/Release/Standalone/WvZz Dynamics.app` |

Plugin identity (from `CMakeLists.txt`):

| Field | Value |
| :--- | :--- |
| CMake target | `WvZzDynamics` |
| Product name | `WvZz Dynamics` |
| Manufacturer code | `WvZz` |
| Plugin code | `WzDy` |
| Formats | `VST3`, `Standalone` |

---

## 6. Documentation Directory

- **[User Manual (USER_MANUAL.md)](USER_MANUAL.md)**: every control with its parameter ID, range, and default, plus presets and usage tips.
- **[Bundle Root Guide (../../README.md)](../../README.md)**: bundle architecture and developer overview.
- **[Developer & Build Manual (../../MANUAL.md)](../../MANUAL.md)**: CMake targets, signing, and tests.
