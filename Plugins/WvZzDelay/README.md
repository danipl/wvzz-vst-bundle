# WvZz Delay

> **Clean, Tape, BBD and Granular Delay of the WvZz Audio Plugin Bundle**  
> Formats: **VST3** & **Standalone** for macOS Universal Binary (`arm64` & `x86_64`). No AU build. Built with JUCE and C++.

---

## 1. What is WvZz Delay?

**WvZz Delay** is a stereo delay effect. It records the input and plays it back later, as one or more repeats.

It is one plugin with a delay module, a grain module, a modulation section, and a master section. The delay has four modes. One of them, `Granular`, builds the repeats from grains. A grain is a short piece of audio.

| # | Module | Card title in the UI | Parameter prefix | Purpose |
| :--- | :--- | :--- | :--- | :--- |
| 1 | Delay | `DELAY` | `dly_` | Stereo delay of 20 ms to 4 s with a filtered feedback loop, diffusion, and ducking. |
| 2 | Grain | `GRAIN` | `grain_` | Replays the delayed audio as up to 32 grains. Used only in `Granular` mode. |
| - | Modulation | `MOD & X/Y` | `lfo1_`, `lfo2_`, `xy_` | Two LFOs and an X/Y pad that move 13 targets. |
| - | Master | `MASTER` | `input_gain`, `master_` | Input gain, dry/wet mix, volume, mono bass, output clip, bypass. |

The plugin exposes 45 parameters. The [User Manual](USER_MANUAL.md) lists each one with its ID, range, and default.

### The Four Modes

| Mode | Sound | What happens when the time changes |
| :--- | :--- | :--- |
| `Clean` | Exact copies of the input. | The plugin crossfades to the new time in 50 ms. The pitch does not change. |
| `Tape` | Saturated repeats with a slow pitch drift (wow and flutter). | The time glides, so the pitch of the repeats bends. |
| `BBD` | Dark repeats, like a BBD (bucket-brigade device, an old analog delay chip). | The time glides, so the pitch of the repeats bends. |
| `Granular` | Repeats built from grains. Grains can be pitch-shifted, reversed, and scattered. | New grains use the new time. Running grains finish at the old time. |

---

## 2. Why WvZz Delay?

### The Problem
1. **Each delay sound needs its own plugin.** A clean delay, a tape echo, an analog delay, and a granular delay are often separate tools.
2. **Repeats are hard to see.** You hear the echoes, but you cannot see when they come or how loud they are.
3. **Feedback can run away.** In many delays, feedback above 100% grows until the output distorts hard.
4. **Presets depend on history.** In many plugins, a preset sounds different after you loaded another one first.

### How WvZz Delay Handles It
- **Four modes in one loop.** All modes share the same time, feedback, filter, diffusion, and ducking controls.
- **Tap display.** The `TAPS` tab draws each repeat on a time axis, for the left and the right channel.
- **Bounded feedback.** Feedback goes up to 110%. A soft clipper in the loop keeps the level limited.
- **Exact time with diffusion.** The diffusion filters do not change the delay time.
- **Ducking.** The repeats get quieter while the input plays and come back when it stops.
- **Bypass with a tail.** The bypass of the Delay card stops recording. The repeats that already exist fade out.
- **Clean preset loading.** A preset first resets every parameter to its default, then applies its own values.
- **No added latency.** The plugin does not report latency to the host. The reported tail follows the time and the feedback.

---

## 3. How It Works (Architecture & DSP)

### Signal Flow

```text
  Main In ──► [Input Gain (-24 to +24 dB)] ──┬──────────────► (dry copy for Master)
                                             │
                    ┌────────────────────────┤
                    │                        │ (dry path inside Delay)
                    ▼                        │
                 ( + ) ◄─────────────────────│──────────────────────┐
                    │                        │                      │
                    ▼                        │                      │
       ┌─────────────────────────┐           │                      │
       │ Delay line (up to 4 s)  │           │                      │
       └────────────┬────────────┘           │                      │
                    ▼                        │                      │
       ┌─────────────────────────┐           │                      │
       │ Tap read by mode:       │           │                      │
       │ Clean / Tape / BBD      │           │                      │
       │ or Grain engine         │           │                      │
       └────────────┬────────────┘           │                      │
                    ▼                        │                      │
       [Diffusion: 2 all-pass filters]       │                      │
                    ▼                        │                      │
       [High-Pass] ──► [Low-Pass]            │                      │
                    ▼                        │                      │
       [Mode color: Tape drive / BBD filter] │                      │
                    │                        │                      │
                    ├──► [x Feedback (0 to 110%)] ──► [Soft clip] ──┘
                    ▼                        │
       [Ducking] ──► [Delay Mix] ◄───────────┘
                    │
                    ▼
       [Master: Dry/Wet ► Volume ► MONO ► CLIP ► Bypass] ──► Stereo Out
```

The diagram shows one channel. The left and the right channel each have their own delay line and loop.

The Modulation section runs first in every audio block. It computes the LFO and X/Y offsets that the other modules read.

### Module Breakdown

#### 1. Delay
- **Time**: 20 ms to 4 s, free or synced to the host tempo with 14 note values. `R Offset` adds up to 50 ms to the right channel.
- **Routing**: `Stereo` keeps left and right separate. `Ping-Pong` sums the input to mono and crosses the feedback, so the repeats alternate left and right.
- **Feedback loop**: tap read, diffusion, high-pass, low-pass, mode color, then back into the delay line.
- **Soft clipper**: a `tanh` curve on the feedback signal. It keeps the loop limited at feedback above 100%.
- **Diffusion**: two all-pass filters per channel smear each repeat. The main tap is shortened by their length, so the total time stays exact.
- **Tape**: `Drive` saturates the repeats. `Wow` adds a 0.5 Hz drift of up to ±1.5 ms and a 6.3 Hz flutter of up to ±0.15 ms.
- **BBD**: an extra two-pole low-pass that gets darker with longer times (5000 Hz down to 1200 Hz). `Wow` works at half depth.
- **Ducking**: follows the dry input (5 ms attack, 250 ms release) and lowers the wet output only, not the loop.
- **Bypass**: stops recording into the delay line. The loop keeps running, so the tail fades out. The dry signal passes at full level.

#### 2. Grain
- **Voices**: 32 fixed voices. When all voices are busy, the next grain is skipped.
- **Grains**: each grain reads the delay line under a Hann window (a smooth fade in and fade out).
- **Controls**: size (10 to 500 ms), density (2 to 60 grains per second), pitch (-24 to +24 semitones), spray (up to 200 ms of random extra delay), reverse chance, stereo spread.
- **Feedback**: the grains go through the same loop. With a pitch shift, each repeat is shifted again.

#### Modulation
- **Two LFOs**: 7 waveforms, free rate (0.02 to 30 Hz) or tempo sync (24 divisions), bipolar depth.
- **X/Y pad**: one target and one bipolar depth per axis.
- **13 targets**: Delay Time, Feedback, LP Cutoff, HP Cutoff, Diffusion, Wow, Delay Mix, Grain Size, Grain Pitch, Grain Spray, Master Mix, LFO 2 Rate, Master Vol.
- **Three views**: the same card shows the X/Y pad, an input/output monitor, or the tap display.

#### Master
- **Input Gain**: applied before the Delay and before the dry copy is taken.
- **Dry/Wet**: blends the dry copy with the output of the Delay.
- **MONO**: sums the signal below 120 Hz to mono.
- **CLIP**: hard clip at 0 dBFS. It is on by default.
- **Bypass**: 10 ms crossfade to the dry signal.

### Audio Buses
- **Input**: stereo.
- **Output**: stereo.
- **Sidechain**: none.

### Latency and Tail
- **Latency**: none reported.
- **Tail**: the time until the repeats are 60 dB quieter. It is between one delay time and 60 seconds. At 100% feedback or more it is 60 seconds.

---

## 4. Factory Presets

The plugin ships with 10 factory presets:

1. Clean Quarter Note
2. Dotted Eighth Ping-Pong
3. Slapback Tape
4. Worn Tape Echo
5. Dark BBD Chorus Echo
6. Dub Feedback Wash
7. Diffused Ambient Trail
8. Ducked Vocal Throw
9. Granular Cloud
10. Octave Shimmer Ladder

The plugin starts on preset 1. The [User Manual](USER_MANUAL.md#5-factory-presets) describes the use case and the mode of each preset.

---

## 5. Build & Install

Run these commands from the repository root. They need CMake and Ninja.

```bash
make delay-vst3              # Build only the WvZz Delay VST3
make delay-standalone        # Build only the WvZz Delay Standalone app
make vst3                    # Build all VST3 bundles in the repository
make test                    # Build and run the DSP tests
make install                 # Copy all VST3 bundles to ~/Library/Audio/Plug-Ins/VST3/
make install-system          # Copy all VST3 bundles to /Library/Audio/Plug-Ins/VST3/ (needs sudo)
make status                  # Show which WvZz plugins are installed
```

Build outputs:

| Format | Path |
| :--- | :--- |
| VST3 | `build/WvZzDelay_artefacts/Release/VST3/WvZz Delay.vst3` |
| Standalone | `build/WvZzDelay_artefacts/Release/Standalone/WvZz Delay.app` |

Plugin identity (from `CMakeLists.txt`):

| Field | Value |
| :--- | :--- |
| CMake target | `WvZzDelay` |
| Product name | `WvZz Delay` |
| Manufacturer code | `WvZz` |
| Plugin code | `WzDl` |
| Formats | `VST3`, `Standalone` |

---

## 6. Documentation Directory

- **[User Manual (USER_MANUAL.md)](USER_MANUAL.md)**: every control with its parameter ID, range, and default, plus presets and usage tips.
- **[Bundle Root Guide (../../README.md)](../../README.md)**: bundle architecture and developer overview.
- **[Developer & Build Manual (../../MANUAL.md)](../../MANUAL.md)**: CMake targets, signing, and tests.
