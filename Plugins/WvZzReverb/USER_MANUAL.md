# WvZz Reverb - User Manual

> Official User Guide for **WvZz Reverb**, part of the **WvZz Audio Plugin Bundle**.

---

## Table of Contents
1. [Overview & Quick Start](#1-overview--quick-start)
2. [Interface Layout & Navigation](#2-interface-layout--navigation)
3. [Module Parameters Guide](#3-module-parameters-guide)
   - [Reverb](#reverb)
   - [Early Reflections](#early-reflections)
4. [Global Controls](#4-global-controls)
   - [Top Header Bar](#top-header-bar)
   - [Modulation: LFO 1 & LFO 2](#modulation-lfo-1--lfo-2)
   - [Modulation: X/Y Pad, Monitor & Decay](#modulation-xy-pad-monitor--decay)
   - [Master Section](#master-section)
5. [Factory Presets](#5-factory-presets)
6. [Signal Flow & Routing](#6-signal-flow--routing)
7. [Usage Tips](#7-usage-tips)
8. [Troubleshooting & FAQ](#8-troubleshooting--faq)

---

## 1. Overview & Quick Start

**WvZz Reverb** is a stereo reverb plugin for macOS (`VST3` and `Standalone`). A reverb adds the sound of a space to the input: first a few separate echoes, then a dense tail that fades out.

The reverb has three modes: `Plate`, `Hall`, and `Shimmer`.

Three terms appear in the whole manual:

- **Early reflections**: the first separate echoes of a room.
- **Pre-delay**: the gap before the reverb tail starts.
- **Tail**: the dense part of the reverb that fades out after the early reflections.

The plugin exposes 41 parameters: 13 for Reverb, 4 for Early Reflections, 18 for Modulation, and 6 for Master.

### 60-Second Quick Start
1. Load **WvZz Reverb** as an insert effect on a track or bus.
2. Pick a preset in the **Preset Selector** in the header:
   - Try `Bright Vocal Plate` on a vocal.
   - Try `Concert Hall` on strings, piano, or a full mix bus.
   - Try `Octave Shimmer` on a pad or a sustained guitar chord.
3. Set **Decay** for the length of the tail in seconds.
4. Set **Mix** for the level of the reverb.
5. Click the `DECAY` tab in the center column to see the tail on a time axis.
6. Change **Mode** to hear the same settings in another reverb type.
7. Match the level with **Volume** in the **MASTER** card.

---

## 2. Interface Layout & Navigation

The window has a header bar and three columns.

```text
┌──────────────────────────────────────────────────────────────────────────────────┐
│ [LOGO] EARLY REFLECTIONS -> REVERB (FDN) -> MASTER  [<][PRESET][>][DICE][BYPASS] │
├─────────────────────────┬──────────────────────────┬─────────────────────────────┤
│       LEFT COLUMN       │      CENTER COLUMN       │        RIGHT COLUMN         │
│                         │                          │                             │
│  ┌───────────────────┐  │  ┌────────────────────┐  │  ┌───────────────────────┐  │
│  │      REVERB       │  │  │ [X/Y PAD][MONITOR] │  │  │   EARLY REFLECTIONS   │  │
│  │ Mode, Interval    │  │  │ [DECAY]            │  │  │ (Size, Diffusion,     │  │
│  │ (mode hint)       │  │  │ X/Y pad, signal    │  │  │  Level)               │  │
│  │                   │  │  │ monitor, or decay  │  │  │                       │  │
│  │ Pre-Delay, Size,  │  │  │ display            │  │  │                       │  │
│  │ Decay, Mix        │  │  └────────────────────┘  │  │                       │  │
│  │                   │  │  ┌────────────────────┐  │  │                       │  │
│  │ Low Damp,         │  │  │       LFO 1        │  │  │                       │  │
│  │ High Damp,        │  │  │ (Wave, SYNC, Rate, │  │  └───────────────────────┘  │
│  │ Diffusion,        │  │  │  Sync Div, Depth,  │  │  ┌───────────────────────┐  │
│  │ Tail Mod          │  │  │  Target)           │  │  │        MASTER         │  │
│  │                   │  │  ├────────────────────┤  │  │ (In Gain, Dry/Wet,    │  │
│  │ Width, Shimmer    │  │  │       LFO 2        │  │  │  Volume, CLIP, MONO)  │  │
│  └───────────────────┘  │  │ (same controls)    │  │  └───────────────────────┘  │
│                         │  └────────────────────┘  │                             │
└─────────────────────────┴──────────────────────────┴─────────────────────────────┘
```

- **Window size**: the default size is 1080 × 680 px.
- **Resizing**: drag the bottom-right corner. The window scales from 810 × 510 px to 1920 × 1200 px and keeps its aspect ratio.
- **Left column**: the `REVERB` card uses the full height. It has a bypass toggle in its header and a **`?`** help button.
- **Right column**: the `EARLY REFLECTIONS` card with a bypass toggle and a **`?`** help button, and the `MASTER` card below it.
- **Mode hint**: a line of text in the `REVERB` card describes the current mode.
- **Visual feedback**: the `REVERB` card dims the controls that the current mode does not use.

| Control | Dimmed when |
| :--- | :--- |
| **Shimmer** | **Mode** is `Plate` or `Hall`. |
| **Interval** | **Mode** is `Plate` or `Hall`. |

### The Three Center Tabs

The top of the center column has three tabs. The two LFO panels below stay visible in every tab.

| Tab | Card title | What it shows |
| :--- | :--- | :--- |
| `X/Y PAD` | `MOD & X/Y` | The X/Y pad with its two targets and two depth knobs. |
| `MONITOR` | `MONITOR` | A display that compares the input and the output signal. The plugin opens on this tab. |
| `DECAY` | `DECAY` | The decay display (see below). |

### The DECAY Tab

The `DECAY` tab draws how the reverb fades. Time runs from left to right. Level runs from 0 dB at the top to -60 dB at the bottom.

- **Header line, left**: the mode, for example `HALL`.
- **Header line, right**: the decay time in seconds and the pre-delay in ms, for example `DECAY 2.50 s   PRE 20 ms`.
- **White bar at 0**: the dry sound.
- **Gap after the white bar**: the pre-delay. The three decay lines start where the gap ends.
- **Green block**: the early reflections. It starts at time 0 and is as wide as the echo pattern (16 to 80 ms). It is hidden when the Early Reflections card is bypassed.
- **Thick cyan line (`MID`)**: the decay of the middle frequencies. It reaches -60 dB at **Pre-Delay** + **Decay**.
- **Amber line (`LOW`)**: the decay of the lows, calculated at 100 Hz. It gets steeper when you raise **Low Damp**.
- **Violet line (`HIGH`)**: the decay of the highs, calculated at 8 kHz. It gets steeper when you lower **High Damp**.
- **`+SHIM` label**: appears in `Shimmer` mode when **Shimmer** is above 0.
- **Green line at the bottom**: the current wet level. It grows to the right when the reverb gets louder.

The time axis is as long as **Pre-Delay** + **Decay**, plus 15%. It is at least 0.5 seconds and at most 30 seconds.

> **Note**: the three decay lines are calculated from the settings. They do not measure the audio. Only the green line at the bottom follows the real signal.

> **Note**: the decay lines do not include the shimmer path. In `Shimmer` mode the real tail can last longer than the cyan line shows.

> **Note**: when the Reverb card is bypassed, the display is dimmed and the header shows `(BYPASSED: TAIL ONLY)`.

---

## 3. Module Parameters Guide

Parameter IDs are the names a host shows for automation.

---

### Reverb

Card title: `REVERB`. This module makes the reverb tail.

It is built on one FDN. FDN means Feedback Delay Network: a set of delay lines that feed each other through a mixing matrix. This plugin uses 8 delay lines. The mixing matrix sends the output of every line into every line. After a short time the echoes are so dense that you hear one smooth tail.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `rev_bypass` | On / Off | Off | Bypasses the module. The tail that already exists fades out. |
| Mode | `rev_mode` | Plate / Hall / Shimmer | Hall | Reverb type. See [Modes](#modes). |
| Interval | `rev_interval` | +1 Octave / +Fifth / +Octave+Fifth / +2 Octaves / -1 Octave | +1 Octave | Pitch shift of the shimmer path. Used only in `Shimmer` mode. |
| Pre-Delay | `rev_predelay` | 0 – 250 ms | 20 ms | Gap before the reverb tail starts. A longer gap keeps the dry sound clear. |
| Size | `rev_size` | 0.0 – 1.0 | 0.5 | Size of the space. It scales the delay lines from half to 1.5 times their length. It does not change the decay time. |
| Decay | `rev_decay` | 0.2 – 20 s | 2.5 s | Time the tail needs to fall by 60 dB. |
| Mix | `rev_mix` | 0.0 – 1.0 | 0.3 | Level of the reverb. The dry signal gets quieter as **Mix** goes up. At 0.0 the output is exactly the input. |
| Low Damp | `rev_lodamp` | 20 – 1000 Hz | 100 Hz | Frequencies below this value fade faster than the rest. At 20 Hz the damping is off. |
| High Damp | `rev_hidamp` | 1000 – 20000 Hz | 6000 Hz | Frequencies above this value fade faster than the rest. At 20000 Hz the damping is off. |
| Diffusion | `rev_diffusion` | 0.0 – 1.0 | 0.7 | Smears the input before it enters the network. Low values give separate echoes at the start. High values give a smooth start. |
| Tail Mod | `rev_mod` | 0.0 – 1.0 | 0.25 | Slow movement of the delay lines. It removes metallic ringing. High values add a light chorus sound. |
| Width | `rev_width` | 0.0 – 1.0 | 1.0 | Stereo width of the tail. 0.0 is mono. 1.0 is full stereo. |
| Shimmer | `rev_shimmer` | 0.0 – 1.0 | 0.5 | Amount of pitch-shifted tail that goes back into the network. Used only in `Shimmer` mode. |

**Intervals in semitones**: `+1 Octave` is +12. `+Fifth` is +7. `+Octave+Fifth` is +19. `+2 Octaves` is +24. `-1 Octave` is -12.

#### Signal Path Inside the Reverb
The input passes these steps:

1. **Sum**: the plugin adds the early reflections to the input.
2. **Pre-delay**: the sum is delayed by **Pre-Delay**.
3. **Shimmer return**: in `Shimmer` mode, the pitch-shifted tail is added here.
4. **Input diffusion**: four all-pass filters per channel. An all-pass filter changes the timing of frequencies, not their level.
5. **Network**: the 8 delay lines with damping, decay gain, and the mixing matrix.
6. **Width**, then the early reflections are added, then **Mix**.

#### Decay and Size
- **Decay** is a time in seconds. After this time the tail is 60 dB quieter.
- Each delay line has its own gain. The plugin computes it from the length of the line and from **Decay**. A long line gets a lower gain per pass than a short one.
- So **Size** changes the character of the space, not the length of the tail. A small size sounds dense and tight. A large size sounds open, with more time between echoes.
- In tests, the measured decay was within 3% to 6% of the setting in `Plate` and `Hall` mode.
- The plugin lowers the level that enters the network when **Decay** is long. A 10-second tail is not louder than a 1-second tail.

#### Damping
- Each of the 8 delay lines splits off the highs above **High Damp** and the lows below **Low Damp**.
- On every trip through a line, a share of those two ranges is removed.
- The share depends on the line length. A short line removes less per trip than a long line. So the loss is the same per second in Plate, Hall, and Shimmer, and at every **Size**.
- The result: frequencies far inside a damped range fade in about 35% of the **Decay** time. With **Decay** at 3 s, they fade in about 1 s.
- With **High Damp** at 20000 Hz and **Low Damp** at 20 Hz, damping is off. All frequencies then fade at the **Decay** time.

#### Tail Modulation
- 4 of the 8 delay lines change their length slowly. Each one has its own rate: 0.31, 0.47, 0.71, and 0.97 Hz.
- At **Tail Mod** 1.0 the change is up to ±0.25 ms.
- A fixed network can ring at a few fixed frequencies. The movement blurs these frequencies, so the tail sounds smoother.

#### Bypass
The header toggle of the `REVERB` card is a soft bypass:

1. The plugin stops feeding new input into the network. This takes about 30 ms.
2. The network keeps running, so the tail that already exists fades out at its normal rate.
3. The dry signal returns to full level.

> **Note**: this toggle also fades out the early reflections. Only the tail that is already sounding keeps playing until it has decayed.

#### Modes

**Plate**
- A plate reverb is a classic studio reverb made with a large metal sheet. The sound is dense and bright.
- The delay lines are short: 11.3 to 38.3 ms at **Size** 0.5.
- The input diffusion is strong, so the tail starts at once.
- **Shimmer** and **Interval** have no effect.

**Hall**
- The sound of a large room. The tail builds up slowly.
- The delay lines are long: 37.3 to 97.3 ms at **Size** 0.5.
- The input diffusion is a little weaker than in `Plate` mode.
- **Shimmer** and **Interval** have no effect.

**Shimmer**
- The same network as `Hall`, plus a shimmer path.
- The shimmer path takes the output of the network and shifts its pitch by **Interval**. A grain pitch shifter does this. It cuts the sound into short pieces (grains) and plays them faster or slower.
- The shifted sound passes a 9 kHz low-pass filter and a limiter. Then it goes back into the network at the **Shimmer** amount.
- The tail goes through this path again and again. With `+1 Octave`, the tail climbs one more octave on each pass.
- With **Shimmer** at 0.0 this mode sounds like `Hall`.

#### Mode Change
When you change **Mode**, the plugin does this:

1. It fades the wet signal out in about 30 ms.
2. It clears the network and sets the delay lines of the new mode.
3. It fades the wet signal in again in about 30 ms.

The old tail is lost. The dry signal and the early reflections are not interrupted.

---

### Early Reflections

Card title: `EARLY REFLECTIONS`. This module makes the first separate echoes of a room. They tell the ear how big the room is and how close the walls are.

The module makes 8 echoes per channel. The left and the right channel use different echo times and levels, so the result is stereo. Later echoes are quieter than earlier ones.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `er_bypass` | On / Off | Off | Turns the early reflections off. The reverb tail then gets the dry input only. |
| Size | `er_size` | 0.0 – 1.0 | 0.5 | Length of the echo pattern. 0.0 is 16 ms. 1.0 is 80 ms. The default is 48 ms. |
| Diffusion | `er_diffusion` | 0.0 – 1.0 | 0.5 | Smears the echoes with two all-pass filters. 0.0 gives sharp, separate echoes. |
| Level | `er_level` | 0.0 – 1.0 | 0.5 | Level of the early reflections. |

The early reflections are used in two places:

1. **In the wet signal.** You hear them together with the tail. Reverb **Mix** also scales them.
2. **As input of the tail.** They go into the reverb before the pre-delay. So a higher **Level** also makes the tail a little louder and denser.

> **Note**: the early reflections are not delayed by **Pre-Delay**. They start right after the dry sound. The tail follows after the pre-delay.

> **Note**: with Reverb **Mix** at 0.0 you hear no early reflections, at any **Level**.

---

## 4. Global Controls

### Top Header Bar

- **Logo and subtitle**: shows `REVERB  //  PLATE, HALL & SHIMMER`.
- **Signal flow label**: shows `EARLY REFLECTIONS -> REVERB (FDN) -> MASTER`.
- **Preset Selector**: lists the 10 factory presets. The `<` and `>` buttons step through them.
- **DICE**: sets 13 knobs to random values inside safe limits. See below.
- **BYPASS**: master bypass. See `master_bypass` in the [Master Section](#master-section).

**What DICE changes**: **Pre-Delay**, **Size**, **Decay**, **Low Damp**, **High Damp**, **Diffusion**, **Tail Mod**, **Width**, **Shimmer**, and **Mix** on the Reverb card. **Size**, **Diffusion**, and **Level** on the Early Reflections card.

**What DICE does not change**: the two card bypass toggles, **Mode**, **Interval**, Modulation, and Master. DICE keeps **Decay** between about 0.6 and 11.5 seconds, and **Mix** between 0.2 and 0.5.

---

### Modulation: LFO 1 & LFO 2

The two LFOs sit in the lower part of the center column. An LFO (low-frequency oscillator) is a slow repeating wave that moves a parameter for you. Each LFO moves one target.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Wave (LFO 1) | `lfo1_wave` | Sine / Triangle / Saw Down / Saw Up / Square / Sample & Hold / Smooth Random | Sine | Waveform of LFO 1. |
| SYNC (LFO 1) | `lfo1_sync` | On / Off | Off | Off: free rate in Hz. On: rate follows the host tempo. |
| Rate (LFO 1) | `lfo1_rate` | 0.02 – 30 Hz | 1.0 Hz | Speed of LFO 1 when **SYNC** is off. |
| Sync Div (LFO 1) | `lfo1_sync_div` | 24 divisions (see below) | 1/4 | Cycle length of LFO 1 when **SYNC** is on. |
| Target (LFO 1) | `lfo1_target` | 15 choices (see below) | None | Parameter that LFO 1 moves. |
| Depth (LFO 1) | `lfo1_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the modulation. |
| Wave (LFO 2) | `lfo2_wave` | Same 7 waveforms | Sine | Waveform of LFO 2. |
| SYNC (LFO 2) | `lfo2_sync` | On / Off | Off | Off: free rate in Hz. On: rate follows the host tempo. |
| Rate (LFO 2) | `lfo2_rate` | 0.02 – 30 Hz | 1.0 Hz | Speed of LFO 2 when **SYNC** is off. |
| Sync Div (LFO 2) | `lfo2_sync_div` | 24 divisions (see below) | 1/4 | Cycle length of LFO 2 when **SYNC** is on. |
| Target (LFO 2) | `lfo2_target` | 15 choices (see below) | None | Parameter that LFO 2 moves. |
| Depth (LFO 2) | `lfo2_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the modulation. |

**Sync divisions**: `4/1`, `2/1`, `1/1`, `1/2`, `1/4`, `1/8`, `1/16`, `1/32`. Each one also exists as dotted (`D`) and triplet (`T`).

**Targets**: `None` plus 14 targets.

| # | Target | What moves | Reach at full depth |
| :--- | :--- | :--- | :--- |
| 1 | `Pre-Delay` | Reverb **Pre-Delay** | ±100 ms |
| 2 | `Size` | Reverb **Size** | ±1.0 |
| 3 | `Decay` | Reverb **Decay** | 2 octaves: from a quarter to four times the decay |
| 4 | `High Damp` | Reverb **High Damp** | 3 octaves up or down |
| 5 | `Low Damp` | Reverb **Low Damp** | 3 octaves up or down |
| 6 | `Diffusion` | Reverb **Diffusion** | ±1.0 |
| 7 | `Tail Mod` | Reverb **Tail Mod** | ±1.0 |
| 8 | `Width` | Reverb **Width** | ±1.0 |
| 9 | `Shimmer` | Reverb **Shimmer** | ±1.0 |
| 10 | `Reverb Mix` | Reverb **Mix** | ±1.0 |
| 11 | `ER Level` | Early Reflections **Level** | ±1.0 |
| 12 | `Master Mix` | Master **Dry/Wet** | ±1.0 |
| 13 | `LFO 2 Rate` | Speed of LFO 2 | 3 octaves up or down |
| 14 | `Master Vol` | Master **Volume** | ±24 dB |

The modulated value always stays inside the range of its parameter. The knob on screen does not move.

`LFO 2 Rate` works only as the target of LFO 1.

**Tempo sync**: while the host plays, a synced LFO is locked to the host beat position. While the host is stopped, it runs free at the host tempo.

**Size and Pre-Delay as targets**: these two change delay lengths, so the pitch of the tail moves up and down with the LFO. Use a small depth for a chorus-like movement.

**Not available as targets**: **Mode**, **Interval**, and the **Size** and **Diffusion** of the Early Reflections card.

---

### Modulation: X/Y Pad, Monitor & Decay

The top of the center column has three tabs: `X/Y PAD`, `MONITOR`, and `DECAY`.

**X/Y PAD tab** (card title `MOD & X/Y`): drag the puck to move two targets at once. The center of the pad means no change.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Puck, horizontal | `xy_x` | 0.0 – 1.0 | 0.5 | Horizontal position of the puck. |
| Puck, vertical | `xy_y` | 0.0 – 1.0 | 0.5 | Vertical position of the puck. |
| X Target | `xy_x_target` | Same 15 choices as the LFOs | None | Parameter that the horizontal axis moves. |
| Y Target | `xy_y_target` | Same 15 choices as the LFOs | None | Parameter that the vertical axis moves. |
| X Depth | `xy_x_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the horizontal modulation. |
| Y Depth | `xy_y_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the vertical modulation. |

**MONITOR tab** (card title `MONITOR`): replaces the pad with a display that compares the input and the output signal. The plugin opens on this tab.

**DECAY tab** (card title `DECAY`): replaces the pad with the decay display. See [The DECAY Tab](#the-decay-tab).

---

### Master Section

Card title: `MASTER`. The BYPASS button is in the header bar.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| In Gain | `input_gain` | -24 – +24 dB | 0 dB | Gain before the Early Reflections and the Reverb. It also changes the dry signal. |
| Dry/Wet | `master_mix` | 0.0 – 1.0 | 1.0 | 0.0 is the dry signal only. 1.0 is the output of the Reverb only. |
| Volume | `master_vol` | -36 – +12 dB | 0 dB | Output level. Applied before MONO and CLIP. |
| MONO | `master_mono_bass` | On / Off | Off | Sums everything below 120 Hz to mono. It removes the left/right difference in the lows and leaves mono content unchanged. |
| CLIP | `master_clip` | On / Off | On | Hard clip at 0 dBFS on the output. |
| BYPASS (header) | `master_bypass` | On / Off | Off | Crossfades to the dry signal in 10 ms. |

The card also shows peak and RMS output meters.

> **Note**: the plugin has two mix controls. Reverb **Mix** sets the level of the reverb. Master **Dry/Wet** is 1.0 by default and passes the Reverb output unchanged. In most cases, use Reverb **Mix** only.

> **Note**: the plugin has two bypass controls for the tail. The toggle of the `REVERB` card lets the tail fade out. **BYPASS** in the header cuts the tail in 10 ms.

---

## 5. Factory Presets

WvZz Reverb has 10 factory presets. The plugin starts on preset 1.

| # | Preset | Use case | Mode |
| :--- | :--- | :--- | :--- |
| 1 | **Bright Vocal Plate** | A clear plate for lead vocals. 1.8 s decay, 35 ms pre-delay, highs open up to 11 kHz. | Plate |
| 2 | **Tight Drum Plate** | A short plate for snares and drum buses. 0.8 s decay, 8 ms pre-delay, small size. | Plate |
| 3 | **Dark Studio Plate** | A warm plate for guitars, keys, and dark vocals. 2.4 s decay, **High Damp** at 3500 Hz. | Plate |
| 4 | **Small Room** | A small space that puts dry tracks in a room. 0.6 s decay, smallest size, strong early reflections. | Hall |
| 5 | **Concert Hall** | A classic hall for strings, piano, and orchestral parts. 3.2 s decay, 30 ms pre-delay. | Hall |
| 6 | **Cathedral** | A very large space for choirs, organs, and slow parts. 8 s decay, 60 ms pre-delay, largest size. | Hall |
| 7 | **Infinite Pad Wash** | A very long, moving wash behind pads and drones. 18 s decay, no pre-delay, full diffusion. | Hall |
| 8 | **Octave Shimmer** | A bright tail that climbs for pads and clean guitars. 5 s decay, `+1 Octave`, **Shimmer** at 0.6. | Shimmer |
| 9 | **Fifth Shimmer Choir** | A choir-like tail for sustained chords. 7 s decay, `+Octave+Fifth`, **Shimmer** at 0.75. | Shimmer |
| 10 | **Sub Octave Cavern** | A dark, deep tail for sound design and low drones. 6 s decay, `-1 Octave`, **High Damp** at 3000 Hz. | Shimmer |

### How a Preset Loads
1. The plugin sets every parameter to its default. This includes Early Reflections, Modulation, and Master.
2. The plugin applies the values that the preset stores.

So a preset always sounds the same, whatever you loaded before.

The selected preset is saved with the plugin state. After you reopen a project, the Preset Selector shows the same preset again.

> **Note**: loading a preset also resets **In Gain**, **Volume**, **Dry/Wet**, the LFOs, and the X/Y pad. No factory preset uses LFO 1, LFO 2, or the X/Y pad.

> **Note**: `Infinite Pad Wash` is not endless. Its decay is 18 seconds. The longest possible decay is 20 seconds.

---

## 6. Signal Flow & Routing

```text
[Main In] ──► [Input Gain] ──┬──────────────► (dry copy for Master)
                             │
                             ├──► [Early Reflections] ──► (auxiliary bus) ──┐
                             │      8 echoes ► 2 all-pass ► Level           │
                             ▼                                              │
                    [Reverb]                                                │
                      dry ──────────────────────────────────────────┐       │
                      input + early reflections ◄───────────────────│───────┤
                             │                                      │       │
                             ▼                                      │       │
                      [Pre-Delay]                                   │       │
                             ▼                                      │       │
                      ( + ) ◄──── [Shimmer path] ◄─────────┐        │       │
                             ▼                             │        │       │
                      [Input diffusion]                    │        │       │
                             ▼                             │        │       │
                      [Network: 8 delay lines,             │        │       │
                       damping, decay gain, matrix] ───────┘        │       │
                             ▼                                      │       │
                      [Width]                                       │       │
                             ▼                                      │       │
                      ( + ) ◄── early reflections ◄─────────────────│───────┘
                             ▼                                      ▼
                      [Mix with the dry signal] ◄───────────────────┘
                             │
                             ▼
                    [Master: Dry/Wet ► Volume ► MONO ► CLIP ► Bypass]
                             │
                             ▼
                        [Stereo Out]
```

- The Early Reflections module does not change the main signal. It writes its echoes to an auxiliary bus (a second internal signal path). The Reverb module reads that bus.
- LFO 1, LFO 2, and the X/Y pad are computed first in every audio block.
- The left input feeds 4 of the 8 delay lines. The right input feeds the other 4. The mixing matrix spreads both over all lines.
- The shimmer path is active only in `Shimmer` mode: grain pitch shifter, 9 kHz low-pass, limiter, then the **Shimmer** amount.
- The plugin has a stereo input and a stereo output. It has no sidechain input.

### Latency and Tail
- **Latency**: the plugin reports no latency to the host. The dry signal is not delayed.
- **Tail**: the tail is the time the plugin still makes sound after the input stops. The plugin reports it to the host.
- **Tail length**: **Pre-Delay** + **Decay** + 0.1 seconds. The extra 0.1 seconds covers the early reflections.
- **Shortest tail**: 0.3 seconds, at 0 ms pre-delay and 0.2 s decay.
- **Longest tail**: 20.35 seconds, at 250 ms pre-delay and 20 s decay.
- **Example**: with the default values (20 ms and 2.5 s), the tail is 2.62 seconds.
- **Reverb bypass**: the toggle of the `REVERB` card lets the tail fade out. See [Bypass](#bypass).

---

## 7. Usage Tips

- **Start with Mode, Decay, and Mix.** These three controls set the basic reverb. Add the other controls after that.
- **Use the decay display.** Open the `DECAY` tab and turn **Decay**, **Low Damp**, and **High Damp**. You see how each one changes the tail.
- **Keep vocals clear with Pre-Delay.** A value of 30 to 60 ms separates the words from the tail.
- **Darken the tail, not the dry sound.** Lower **High Damp** to 3000 – 5000 Hz. The reverb then sits behind the dry sound.
- **Keep the low end clean.** Raise **Low Damp** to 200 – 300 Hz on a full mix or a bass-heavy source.
- **Choose the mode by the source.** `Plate` works well on vocals and drums. `Hall` works well on pads, strings, and slow parts.
- **Use Size for character.** A small **Size** with a long **Decay** gives a dense, tight tail. A large **Size** gives a more open tail.
- **Make a small room.** Use `Hall` mode, a **Decay** below 1 second, a small **Size**, and a high Early Reflections **Level**.
- **Move the dry sound closer or farther.** More Early Reflections **Level** and less tail sounds close. More tail and a long **Pre-Delay** sounds far.
- **Start Shimmer low.** Set **Shimmer** to about 0.3 and raise it slowly. High values with a long **Decay** build a dense, bright cloud.
- **Pick the interval for the music.** `+1 Octave` and `+2 Octaves` fit every chord. `+Fifth` and `+Octave+Fifth` add a new note to the chord.
- **Raise Tail Mod on long decays.** A value of 0.4 to 0.6 keeps a long tail smooth. Lower it for a piano or other sounds with a fixed pitch.
- **Use the plugin on a send.** Set Reverb **Mix** to 1.0 so the send returns only the reverb.
- **Narrow the tail in a busy mix.** Lower **Width**, or turn **MONO** on if a wide tail makes the bass unstable.
- **Save your own settings in the host.** Loading a factory preset resets every parameter.

---

## 8. Troubleshooting & FAQ

**Q: Why is there no AU (Audio Unit) version?**  
A: WvZz Reverb is built as VST3 and Standalone only. Use a VST3 host or the Standalone app.

**Q: Why does damping change the decay of the lows and highs, but the Decay knob still shows the same time?**  
A: **Decay** is the decay time of the middle frequencies. Damping removes a share of the highs and lows on every trip through a delay line, so those ranges fade faster: in about 35% of the **Decay** time when they are far beyond the **High Damp** or **Low Damp** frequency. The `DECAY` tab shows all three lines.

**Q: Why does a Size change bend the pitch for a moment?**  
A: **Size** changes the length of the delay lines. A sudden jump would make a click. So each line glides to its new length in about 0.1 seconds. While a line gets shorter, the sound in it plays back faster, so the pitch goes up. While it gets longer, the pitch goes down. The pitch returns to normal when the glide ends. **Pre-Delay** glides in the same way.

**Q: What stops shimmer from running away?**  
A: Three things. First, the pitch-shifted sound passes a limiter (a `tanh` curve) before it returns to the network. The limiter has a fixed maximum output, whatever the input level is. Second, a 9 kHz low-pass filter removes the highest part on every pass. Third, each delay line always has a gain below 1, so the network loses energy on every pass. In a test with 30 seconds of full-scale noise, **Decay** at 20 s, and **Shimmer** at 1.0, the peak was 3.4 and did not keep growing.

**Q: Why does a mode change have a short fade?**  
A: `Plate` and `Hall` use delay lines of different lengths. The sound that is still inside the lines does not fit the new lengths. Without a fade you would hear a click or a burst. So the plugin fades the wet signal out in about 30 ms, clears the network, and fades in again. The old tail is lost.

**Q: Why do the Shimmer knob and the Interval box do nothing?**  
A: They work only in `Shimmer` mode. In `Plate` and `Hall` mode they are dimmed.

**Q: Why does the tail in Shimmer mode last longer than the Decay time?**  
A: The shimmer path sends part of the tail back into the network. This adds new sound while the old sound fades. Lower **Shimmer** for a shorter tail.

**Q: Why do I still hear echoes when the Reverb card is bypassed?**  
A: The tail that is already sounding fades out on its own; that is intended. New sound, and the early reflections, are not processed while the `REVERB` card is bypassed. Use **BYPASS** in the header to cut the tail at once.

**Q: Why is the output loud with a loud input and a long decay?**  
A: A long tail adds up many echoes of a steady input. The plugin scales the input of the network to limit this, and **CLIP** in the `MASTER` card limits the output to 0 dBFS. Lower **Mix** or **In Gain** if the output clips.

**Q: Why is there a small pitch movement in the tail?**  
A: **Tail Mod** moves 4 of the 8 delay lines. Set it to 0.0 for a tail with no pitch movement.

**Q: Why did my Volume and LFO settings change when I picked a preset?**  
A: A preset resets every parameter to its default before it applies its values. This includes Master and Modulation.

**Q: Why does the sound change when I move the X/Y pad?**  
A: An axis has a target and a depth. Set **X Depth** and **Y Depth** to 0.0 to turn the pad off.
