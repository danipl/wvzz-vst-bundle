# WvZz Reverb

> **Plate, Hall and Shimmer Reverb of the WvZz Audio Plugin Bundle**  
> Formats: **VST3** & **Standalone** for macOS Universal Binary (`arm64` & `x86_64`). No AU build. Built with JUCE and C++.

---

## 1. What is WvZz Reverb?

**WvZz Reverb** is a stereo reverb effect. A reverb adds the sound of a space to the input: first a few separate echoes, then a dense tail that fades out.

It is one plugin with an early reflections module, a reverb module, a modulation section, and a master section. Early reflections are the first separate echoes of a room. The reverb module makes the tail and has three modes.

| # | Module | Card title in the UI | Parameter prefix | Purpose |
| :--- | :--- | :--- | :--- | :--- |
| 1 | Early Reflections | `EARLY REFLECTIONS` | `er_` | 8 echoes per channel in a pattern of 16 to 80 ms. They are heard in the wet signal and also feed the tail. |
| 2 | Reverb | `REVERB` | `rev_` | The reverb tail: pre-delay, a network of 8 delay lines, damping, width, and a shimmer path. |
| - | Modulation | `MOD & X/Y` | `lfo1_`, `lfo2_`, `xy_` | Two LFOs and an X/Y pad that move 14 targets. |
| - | Master | `MASTER` | `input_gain`, `master_` | Input gain, dry/wet mix, volume, mono bass, output clip, bypass. |

The plugin exposes 41 parameters. The [User Manual](USER_MANUAL.md) lists each one with its ID, range, and default.

### The Three Modes

| Mode | Sound | How it is built |
| :--- | :--- | :--- |
| `Plate` | Dense and bright. The tail starts at once. | Short delay lines (11 to 38 ms) and strong input diffusion. |
| `Hall` | A large space. The tail builds up slowly. | Long delay lines (37 to 97 ms) and a little less input diffusion. |
| `Shimmer` | A hall with a tail that climbs in pitch. | The `Hall` network, plus a pitch shifter that sends the tail back into the network. |

---

## 2. Why WvZz Reverb?

### The Problem
1. **Decay knobs are often not exact.** In many reverbs the real decay time changes when you change the room size.
2. **A tail is hard to see.** You hear the reverb, but you cannot see how long it is or how the damping shapes it.
3. **Long decays get loud.** In many reverbs a long decay is much louder than a short one.
4. **Shimmer can run away.** A pitch shifter inside a feedback loop can grow until the output distorts hard.
5. **Presets depend on history.** In many plugins, a preset sounds different after you loaded another one first.

### How WvZz Reverb Handles It
- **Decay in seconds.** **Decay** is the time the tail needs to fall by 60 dB. In tests, the measured time was within 3% to 6% of the setting in `Plate` and `Hall` mode.
- **Size without a decay change.** **Size** changes the length of the delay lines. The plugin corrects the gain of each line, so the decay time stays the same.
- **Decay display.** The `DECAY` tab draws the level of the tail against time, for low, mid, and high frequencies.
- **Steady level.** The plugin lowers the level that enters the network when **Decay** is long. A long tail is not louder than a short one.
- **Bounded shimmer.** A limiter in the shimmer path limits what the pitch shifter can send back.
- **Bypass with a tail.** The bypass of the Reverb card stops feeding the network. The tail that already exists fades out.
- **Clean preset loading.** A preset first resets every parameter to its default, then applies its own values.
- **No added latency.** The plugin does not report latency to the host. The reported tail follows **Pre-Delay** and **Decay**.

---

## 3. How It Works (Architecture & DSP)

### Signal Flow

```text
  Main In ──► [Input Gain (-24 to +24 dB)] ──┬──────────────► (dry copy for Master)
                                             │
             ┌───────────────────────────────┼────────────────────────────┐
             ▼                               │                            │
  [Early Reflections:                        │                            │
   8 echoes ► 2 all-pass ► Level]            │                            │
             │                               ▼                            │
             ├───────────────────────────► ( + )                          │
             │                               ▼                            │
             │                  [Pre-Delay (0 to 250 ms)]                 │
             │                               ▼                            │
             │                             ( + ) ◄──────────────────┐     │
             │                               ▼                      │     │
             │             [Input diffusion: 4 all-pass filters]    │     │
             │                               ▼                      │     │
             │             ┌───────────────────────────────────┐    │     │
             │             │ Feedback Delay Network (FDN)      │    │     │
             │             │ 8 delay lines                     │    │     │
             │             │ ► damping (low-pass, high-pass)   │    │     │
             │             │ ► decay gain per line             │    │     │
             │             │ ► 8x8 Hadamard mixing matrix      │    │     │
             │             │ ► back into the 8 lines           │    │     │
             │             └─────────────────┬─────────────────┘    │     │
             │                               │                      │     │
             │                               ├──► [Shimmer path] ───┘     │
             │                               │    (Shimmer mode only)     │
             │                               ▼                            │
             │                      [Width (mid/side)]                    │
             │                               ▼                            │
             └───────────────────────────► ( + )                          │
                                             ▼                            │
                                       [Reverb Mix] ◄─────────────────────┘
                                             │              (dry path inside Reverb)
                                             ▼
       [Master: Dry/Wet ► Volume ► MONO ► CLIP ► Bypass] ──► Stereo Out

  Shimmer path: [Grain pitch shifter] ► [9 kHz low-pass] ► [tanh limiter] ► [x Shimmer]
```

The diagram shows one channel. The left input feeds lines 1, 3, 5, and 7 of the network. The right input feeds lines 2, 4, 6, and 8. The mixing matrix then spreads both sides over all 8 lines.

The Modulation section runs first in every audio block. It computes the LFO and X/Y offsets that the other modules read.

### Module Breakdown

#### 1. Early Reflections
- **Pattern**: 8 echoes per channel. The left and the right channel use different echo times and levels, so the two sides sound different.
- **Size**: sets the length of the pattern, from 16 ms to 80 ms.
- **Diffusion**: two all-pass filters per channel smear the echoes. An all-pass filter changes the timing of frequencies, not their level.
- **Output**: the module does not change the main signal. It writes the echoes to an auxiliary bus (a second internal signal path).
- **Two uses**: the Reverb module adds the echoes to its wet signal. It also adds them to the input of the tail, before the pre-delay.

#### 2. Reverb
- **Pre-delay**: the gap before the reverb tail starts. 0 to 250 ms.
- **Input diffusion**: four all-pass filters per channel smear the input before it enters the network.
- **FDN**: Feedback Delay Network. It is a set of delay lines that feed each other through a mixing matrix. This plugin uses 8 lines.
- **Mixing matrix**: an 8x8 Hadamard matrix. It sends the output of every line to every line. It preserves energy, so the matrix adds no level and removes none.
- **Decay**: each line has its own gain, computed from its length and **Decay**. The result is a fall of 60 dB in **Decay** seconds.
- **Line lengths**: `Plate` uses 11.3 to 38.3 ms. `Hall` and `Shimmer` use 37.3 to 97.3 ms. **Size** scales them by 0.5 to 1.5.
- **Damping**: each line removes a share of the highs above High Damp and of the lows below Low Damp on every pass. The share depends on the line length, so the loss is the same per second in every mode: the damped range fades in about 35% of the Decay time. At 20 kHz and 20 Hz the damping is off.
- **Level**: the input of the network is scaled by `sqrt(1 - g^2)`, where `g` is the line gain. This keeps the level steady across the **Decay** range.
- **Tail modulation**: 4 of the 8 lines change their length slowly (0.31 to 0.97 Hz). This removes metallic ringing.
- **Shimmer**: the network output goes through a grain pitch shifter, a 9 kHz low-pass, and a `tanh` limiter, then back into the network. Intervals: +12, +7, +19, +24, or -12 semitones.
- **Mode change**: the wet signal fades out in about 30 ms, the network is cleared, and the wet signal fades in.
- **Width**: mid/side. The plugin scales the difference between left and right. 0.0 is mono.
- **Bypass**: stops feeding the network. The tail fades out at its normal rate. The dry signal passes at full level.
- **Mix**: at 0.0 the output is exactly the input.

#### Modulation
- **Two LFOs**: 7 waveforms, free rate (0.02 to 30 Hz) or tempo sync (24 divisions), bipolar depth.
- **X/Y pad**: one target and one bipolar depth per axis.
- **14 targets**: Pre-Delay, Size, Decay, High Damp, Low Damp, Diffusion, Tail Mod, Width, Shimmer, Reverb Mix, ER Level, Master Mix, LFO 2 Rate, Master Vol.
- **Three views**: the same card shows the X/Y pad, an input/output monitor, or the decay display.

#### Master
- **Input Gain**: applied before the Early Reflections and before the dry copy is taken.
- **Dry/Wet**: blends the dry copy with the output of the Reverb.
- **MONO**: sums the signal below 120 Hz to mono.
- **CLIP**: hard clip at 0 dBFS. It is on by default.
- **Bypass**: 10 ms crossfade to the dry signal.

### Audio Buses
- **Input**: stereo.
- **Output**: stereo.
- **Sidechain**: none.

### Latency and Tail
- **Latency**: none reported.
- **Tail**: **Pre-Delay** + **Decay** + 0.1 seconds. With the default values this is 0.02 + 2.5 + 0.1 = 2.62 seconds.

### Measured Results
The DSP tests in `Tests/DSPTests.cpp` measure the reverb. These are the results of the last run:

| Test | Result |
| :--- | :--- |
| Decay time in `Plate` and `Hall` mode, at **Size** 0.2, 0.5, and 0.9 | Within 3% to 6% of the **Decay** setting. The test limit is 10%. |
| 30 seconds of full-scale noise, **Decay** at 20 s, **Shimmer** at 1.0 | The peak was 3.4 and did not keep growing. The test limit is 8.0. |
| **Mix** at 0.0 | The output equals the input. |

---

## 4. Factory Presets

The plugin ships with 10 factory presets:

1. Bright Vocal Plate
2. Tight Drum Plate
3. Dark Studio Plate
4. Small Room
5. Concert Hall
6. Cathedral
7. Infinite Pad Wash
8. Octave Shimmer
9. Fifth Shimmer Choir
10. Sub Octave Cavern

The plugin starts on preset 1. The [User Manual](USER_MANUAL.md#5-factory-presets) describes the use case and the mode of each preset.

---

## 5. Build & Install

Run these commands from the repository root. They need CMake and Ninja.

```bash
make reverb-vst3             # Build only the WvZz Reverb VST3
make reverb-standalone       # Build only the WvZz Reverb Standalone app
make vst3                    # Build all VST3 bundles in the repository
make test                    # Build and run the DSP tests
make install                 # Copy all VST3 bundles to ~/Library/Audio/Plug-Ins/VST3/
make install-system          # Copy all VST3 bundles to /Library/Audio/Plug-Ins/VST3/ (needs sudo)
make status                  # Show which WvZz plugins are installed
```

Build outputs:

| Format | Path |
| :--- | :--- |
| VST3 | `build/WvZzReverb_artefacts/Release/VST3/WvZz Reverb.vst3` |
| Standalone | `build/WvZzReverb_artefacts/Release/Standalone/WvZz Reverb.app` |

Plugin identity (from `CMakeLists.txt`):

| Field | Value |
| :--- | :--- |
| CMake target | `WvZzReverb` |
| Product name | `WvZz Reverb` |
| Manufacturer code | `WvZz` |
| Plugin code | `WzRv` |
| Formats | `VST3`, `Standalone` |

---

## 6. Documentation Directory

- **[User Manual (USER_MANUAL.md)](USER_MANUAL.md)**: every control with its parameter ID, range, and default, plus presets and usage tips.
- **[Bundle Root Guide (../../README.md)](../../README.md)**: bundle architecture and developer overview.
- **[Developer & Build Manual (../../MANUAL.md)](../../MANUAL.md)**: CMake targets, signing, and tests.
