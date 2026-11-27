# WvZz Modulation

> **Chorus, Phaser, Flanger and Tremolo Processor of the WvZz Audio Plugin Bundle**  
> Formats: **VST3** & **Standalone** for macOS Universal Binary (`arm64` & `x86_64`). No AU build. Built with JUCE 8 and C++20.

---

## 1. What is WvZz Modulation?

**WvZz Modulation** is a modulation effect chain. It adds movement to a sound with a chorus, a phaser, a flanger, and a tremolo.

It is one plugin with four effect modules in a fixed order, plus a modulation section and a master section.

| # | Module | Card title in the UI | Parameter prefix | Purpose |
| :--- | :--- | :--- | :--- | :--- |
| 1 | Chorus | `CHORUS` | `chor_` | Adds 1 to 4 detuned, delayed copies of the signal for width and thickness. |
| 2 | Phaser | `PHASER` | `phs_` | Sweeps notches through the spectrum with 2 to 12 all-pass stages. |
| 3 | Flanger | `FLANGER` | `flg_` | Mixes a short swept delay with the signal, with an optional through-zero mode. |
| 4 | Tremolo | `TREMOLO` | `trem_` | Moves the level up and down, in Classic or Harmonic mode, with auto-pan. |
| - | Modulation | `MOD & X/Y` | `lfo1_`, `lfo2_`, `xy_` | Two LFOs and an X/Y pad that move 14 targets. |
| - | Master | `MASTER` | `input_gain`, `master_` | Input gain, dry/wet mix, volume, mono bass, output clip, bypass. |

The plugin exposes 54 parameters. The [User Manual](USER_MANUAL.md) lists each one with its ID, range, and default.

---

## 2. Why WvZz Modulation?

### The Problem
1. **Modulation needs several plugins.** A typical chain uses a separate chorus, phaser, flanger, and tremolo.
2. **Stereo movement is hard to see.** You hear that left and right move apart, but you cannot see by how much.
3. **Presets depend on history.** In many plugins, a preset sounds different after you loaded another one first.

### How WvZz Modulation Handles It
- **One chain.** All four effects run in one plugin in a fixed order.
- **Every effect has its own bypass.** All four are bypassed by default. A preset turns on only the effects it needs.
- **Phase scope.** The `SCOPE` tab shows the LFO position of the left and right channel for each effect.
- **Clean preset loading.** A preset first resets every parameter to its default, then applies its own values.
- **Shared modulation.** Two extra LFOs and an X/Y pad can move 14 targets across the four effects and the master.
- **No added latency.** The plugin does not report latency to the host. It reports a tail of 2.0 seconds.

---

## 3. How It Works (Architecture & DSP)

### Signal Flow

```text
                  ┌──────────────────────────────┐
  Main In ───────►│ Input Gain (-24 to +24 dB)   │
                  └───────────────┬──────────────┘
                                  │
            ┌─────────────────────┴─────────────────────┐
            │                                           │
            ▼ (main path)                               ▼ (dry copy)
  ┌───────────────────────┐                             │
  │ 1. Chorus             │                             │
  │    1 to 4 voices      │                             │
  └───────────┬───────────┘                             │
              ▼                                         │
  ┌───────────────────────┐                             │
  │ 2. Phaser             │                             │
  │    2 to 12 stages     │                             │
  └───────────┬───────────┘                             │
              ▼                                         │
  ┌───────────────────────┐                             │
  │ 3. Flanger            │                             │
  │    + through-zero     │                             │
  └───────────┬───────────┘                             │
              ▼                                         │
  ┌───────────────────────┐                             │
  │ 4. Tremolo            │                             │
  │    Classic / Harmonic │                             │
  └───────────┬───────────┘                             │
              ▼                                         │
  [Master: Dry/Wet Mix] ◄───────────────────────────────┘
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

A bypassed effect passes the signal through unchanged.

### Module Breakdown

#### 1. Chorus
- **Voices**: 1 to 4 delay taps read one delay line per channel. The voices are spaced evenly over the LFO cycle.
- **Delay and sweep**: the base delay is `Delay` (2 to 30 ms). `Depth` sweeps it by up to ±5 ms.
- **Spread**: shifts the LFO of the right channel by up to 90 degrees.
- **Feedback**: sends the first voice back into the delay line, through a soft limiter.
- **Level**: the wet sum is scaled by `1 / sqrt(voices)`, so more voices do not get louder.

#### 2. Phaser
- **Stages**: 2, 4, 6, 8, or 12 first-order all-pass filters in series, per channel.
- **Sweep**: the LFO moves the filter frequency around `Center`. At full `Depth` the sweep is two octaves up and down.
- **Feedback**: sends the output back to the input, through a soft limiter. Negative values give a different tone.
- **Stereo**: shifts the LFO of the right channel by up to 180 degrees.

#### 3. Flanger
- **Delay**: a short delay (`Center`, 0.5 to 10 ms) swept by `Depth` (0 to 5 ms). The delay never goes below one sample.
- **Amount**: blend of the dry and the delayed signal. At 0.0 the module has no effect.
- **Feedback**: -0.95 to +0.95, through a soft limiter.
- **Stereo**: the LFO of the right channel is always 90 degrees ahead.
- **Through-zero**: delays the dry signal by `Center`. The swept delay then passes through the dry signal, like tape flanging.

#### 4. Tremolo
- **Classic mode**: the LFO lowers the whole signal by up to `Depth`.
- **Harmonic mode**: splits the signal at `X-Over`. The low band and the high band move in opposite phase.
- **Shapes**: `Sine`, `Triangle`, `Square`. The square shape is smoothed over 2 ms to avoid clicks.
- **Stereo**: shifts the LFO of the right channel by up to 180 degrees. At 1.0 the tremolo becomes an auto-pan.

#### Modulation
- **Two LFOs**: 7 waveforms, free rate (0.02 to 30 Hz) or tempo sync (24 divisions), bipolar depth.
- **X/Y pad**: one target and one bipolar depth per axis.
- **14 targets**: Chorus Rate, Chorus Depth, Chorus Mix, Phaser Rate, Phaser Center, Phaser Feedback, Phaser Mix, Flanger Rate, Flanger Amount, Tremolo Rate, Tremolo Depth, Master Mix, LFO 2 Rate, Master Vol.
- **Three views**: the same card shows the X/Y pad, an input/output monitor, or the LFO phase scope.

#### Master
- **Input Gain**: applied before all modules and before the dry copy is taken.
- **Dry/Wet**: blends the dry copy with the processed signal.
- **MONO**: sums the signal below 120 Hz to mono.
- **CLIP**: hard clip at 0 dBFS. It is on by default.
- **Bypass**: 10 ms crossfade to the dry signal.

### Audio Buses
- **Input**: stereo.
- **Output**: stereo.
- **Sidechain**: none.

---

## 4. Factory Presets

The plugin ships with 10 factory presets:

1. Lush Stereo Chorus
2. Dimension Wide
3. Vintage 4-Stage Phaser
4. Deep 12-Stage Sweep
5. Jet Flanger
6. Through-Zero Tape Flange
7. Harmonic Tremolo Shimmer
8. Auto-Pan Pulse
9. Ensemble Strings
10. Psychedelic Swirl

The plugin starts on preset 1. The [User Manual](USER_MANUAL.md#5-factory-presets) describes the use case of each preset.

---

## 5. Build & Install

Run these commands from the repository root. They need CMake and Ninja.

```bash
make modulation-vst3         # Build only the WvZz Modulation VST3
make modulation-standalone   # Build only the WvZz Modulation Standalone app
make vst3                    # Build all VST3 bundles in the repository
make test                    # Build and run the DSP tests
make install                 # Copy all VST3 bundles to ~/Library/Audio/Plug-Ins/VST3/
make install-system          # Copy all VST3 bundles to /Library/Audio/Plug-Ins/VST3/ (needs sudo)
make status                  # Show which WvZz plugins are installed
```

Build outputs:

| Format | Path |
| :--- | :--- |
| VST3 | `build/WvZzModulation_artefacts/Release/VST3/WvZz Modulation.vst3` |
| Standalone | `build/WvZzModulation_artefacts/Release/Standalone/WvZz Modulation.app` |

Plugin identity (from `CMakeLists.txt`):

| Field | Value |
| :--- | :--- |
| CMake target | `WvZzModulation` |
| Product name | `WvZz Modulation` |
| Manufacturer code | `WvZz` |
| Plugin code | `WzMd` |
| Formats | `VST3`, `Standalone` |

---

## 6. Documentation Directory

- **[User Manual (USER_MANUAL.md)](USER_MANUAL.md)**: every control with its parameter ID, range, and default, plus presets and usage tips.
- **[Bundle Root Guide (../../README.md)](../../README.md)**: bundle architecture and developer overview.
- **[Developer & Build Manual (../../MANUAL.md)](../../MANUAL.md)**: CMake targets, signing, and tests.
