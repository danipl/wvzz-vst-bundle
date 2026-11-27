# WvZz Delay - User Manual

> Official User Guide for **WvZz Delay**, part of the **WvZz Audio Plugin Bundle**.

---

## Table of Contents
1. [Overview & Quick Start](#1-overview--quick-start)
2. [Interface Layout & Navigation](#2-interface-layout--navigation)
3. [Module Parameters Guide](#3-module-parameters-guide)
   - [Delay](#delay)
   - [Grain](#grain)
4. [Global Controls](#4-global-controls)
   - [Top Header Bar](#top-header-bar)
   - [Modulation: LFO 1 & LFO 2](#modulation-lfo-1--lfo-2)
   - [Modulation: X/Y Pad, Monitor & Taps](#modulation-xy-pad-monitor--taps)
   - [Master Section](#master-section)
5. [Factory Presets](#5-factory-presets)
6. [Signal Flow & Routing](#6-signal-flow--routing)
7. [Usage Tips](#7-usage-tips)
8. [Troubleshooting & FAQ](#8-troubleshooting--faq)

---

## 1. Overview & Quick Start

**WvZz Delay** is a stereo delay plugin for macOS (`VST3` and `Standalone`). A delay records the input and plays it back later, as one or more repeats.

The delay has four modes: `Clean`, `Tape`, `BBD`, and `Granular`.

The plugin exposes 45 parameters: 15 for Delay, 6 for Grain, 18 for Modulation, and 6 for Master.

### 60-Second Quick Start
1. Load **WvZz Delay** as an insert effect on a track or bus.
2. Pick a preset in the **Preset Selector** in the header:
   - Try `Clean Quarter Note` on a vocal or a lead.
   - Try `Slapback Tape` on a guitar or a vocal.
   - Try `Granular Cloud` on a pad or a sustained chord.
3. Set the delay time. With **SYNC** off, turn **Time**. With **SYNC** on, pick a note value in **Note**.
4. Set **Feedback** for the number of repeats. Set **Mix** for the level of the repeats.
5. Click the `TAPS` tab in the center column to see the repeats on a time axis.
6. Change **Mode** to hear the same settings in another delay type.
7. Match the level with **Volume** in the **MASTER** card.

---

## 2. Interface Layout & Navigation

The window has a header bar and three columns.

```text
┌──────────────────────────────────────────────────────────────────────────────────┐
│ [LOGO] DELAY (+ GRAIN) -> MASTER              [<][PRESET][>][DICE][BYPASS]       │
├─────────────────────────┬──────────────────────────┬─────────────────────────────┤
│       LEFT COLUMN       │      CENTER COLUMN       │        RIGHT COLUMN         │
│                         │                          │                             │
│  ┌───────────────────┐  │  ┌────────────────────┐  │  ┌───────────────────────┐  │
│  │       DELAY       │  │  │ [X/Y PAD][MONITOR] │  │  │         GRAIN         │  │
│  │ Mode, Routing     │  │  │ [TAPS]             │  │  │ (Size, Density,       │  │
│  │ SYNC, Note        │  │  │ X/Y pad, signal    │  │  │  Pitch, Spray,        │  │
│  │                   │  │  │ monitor, or delay  │  │  │  Reverse, Spread)     │  │
│  │ Time, R Offset,   │  │  │ tap display        │  │  │                       │  │
│  │ Feedback, Mix     │  │  └────────────────────┘  │  │                       │  │
│  │                   │  │  ┌────────────────────┐  │  │                       │  │
│  │ High-Pass,        │  │  │       LFO 1        │  │  │                       │  │
│  │ Low-Pass,         │  │  │ (Wave, SYNC, Rate, │  │  └───────────────────────┘  │
│  │ Diffusion,        │  │  │  Sync Div, Depth,  │  │  ┌───────────────────────┐  │
│  │ Ducking           │  │  │  Target)           │  │  │        MASTER         │  │
│  │                   │  │  ├────────────────────┤  │  │ (In Gain, Dry/Wet,    │  │
│  │ Wow, Drive        │  │  │       LFO 2        │  │  │  Volume, CLIP, MONO)  │  │
│  └───────────────────┘  │  │ (same controls)    │  │  └───────────────────────┘  │
│                         │  └────────────────────┘  │                             │
└─────────────────────────┴──────────────────────────┴─────────────────────────────┘
```

- **Window size**: the default size is 1080 × 680 px.
- **Resizing**: drag the bottom-right corner. The window scales from 810 × 510 px to 1920 × 1200 px and keeps its aspect ratio.
- **Left column**: the `DELAY` card uses the full height. It has a bypass toggle in its header and a **`?`** help button.
- **Right column**: the `GRAIN` card with a **`?`** help button, and the `MASTER` card below it.
- **Mode hint**: a line of text in the `DELAY` card describes the current mode.
- **Visual feedback**: the `DELAY` card dims the controls that the current settings do not use.

| Control | Dimmed when |
| :--- | :--- |
| **Time** | **SYNC** is on. |
| **Note** | **SYNC** is off. |
| **Wow** | **Mode** is `Clean` or `Granular`. |
| **Drive** | **Mode** is not `Tape`. |

The `GRAIN` card is never dimmed. Its knobs change the sound only when **Mode** is `Granular`.

### The Three Center Tabs

The top of the center column has three tabs. The two LFO panels below stay visible in every tab.

| Tab | Card title | What it shows |
| :--- | :--- | :--- |
| `X/Y PAD` | `MOD & X/Y` | The X/Y pad with its two targets and two depth knobs. |
| `MONITOR` | `MONITOR` | A display that compares the input and the output signal. The plugin opens on this tab. |
| `TAPS` | `TAPS` | The delay tap display (see below). |

### The TAPS Tab

A tap is one repeat of the delay. The `TAPS` tab draws the repeats on a time axis. Time runs from left to right.

- **Header line, left**: the mode and the routing, for example `TAPE  STEREO`.
- **Header line, right**: the delay time of the left channel in ms, and the feedback in percent.
- **White bar at 0**: the dry signal.
- **Cyan bars above the center line**: the repeats of the left channel (`L`).
- **Amber bars below the center line**: the repeats of the right channel (`R`).
- **Bar position**: when the repeat plays. The labels below the plot show the time in ms or seconds.
- **Bar height**: the level of the repeat. The first repeat has full height. Each next bar is lower by the feedback amount.
- **Ducking**: all bars shrink while ducking lowers the repeats.
- **Green line at the bottom**: the current wet level. It grows to the right when the repeats get louder.

The display shows up to 16 repeats per side. It stops earlier when a repeat falls below 1% of the first one.

The time axis is at least 1 second long. It grows when the repeats need more room.

In `Stereo` routing, both sides show their own row of repeats. With **R Offset** above 0, the amber bars are a little later than the cyan bars.

In `Ping-Pong` routing, the bars alternate: first left, then right, then left again. The display shows up to 16 bars in total.

> **Note**: the bars are calculated from the time and the feedback. They do not measure the audio. Only the green line follows the real signal.

> **Note**: when the Delay card is bypassed, the display is dimmed and the header shows `(BYPASSED: TAIL ONLY)`.

---

## 3. Module Parameters Guide

Parameter IDs are the names a host shows for automation.

---

### Delay

Card title: `DELAY`. This module records the input into a delay line and plays it back after the delay time. Part of the output goes back into the delay line. This is the feedback loop. It makes more than one repeat.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Header toggle | `dly_bypass` | On / Off | Off | Bypasses the module. The repeats that already exist fade out. |
| Mode | `dly_mode` | Clean / Tape / BBD / Granular | Clean | Delay type. See [Modes](#modes). |
| Routing | `dly_routing` | Stereo / Ping-Pong | Stereo | `Stereo` keeps left and right separate. `Ping-Pong` makes the repeats alternate left and right. |
| SYNC | `dly_sync` | On / Off | Off | Off: the time comes from **Time**. On: the time follows the host tempo and **Note**. |
| Note | `dly_sync_div` | 14 note values (see below) | 1/4 | Delay time as a note value. Used only when **SYNC** is on. |
| Time | `dly_time` | 20 – 4000 ms | 350 ms | Delay time. Used only when **SYNC** is off. |
| R Offset | `dly_offset` | 0 – 50 ms | 0 ms | Extra delay time for the right channel. It makes the repeats wider. |
| Feedback | `dly_feedback` | 0.0 – 1.1 | 0.4 | How much of each repeat goes back into the delay. 1.0 is 100%. 1.1 is 110%. |
| Mix | `dly_mix` | 0.0 – 1.0 | 0.35 | Level of the repeats. The dry signal gets quieter as **Mix** goes up. At 1.0 you hear only the repeats. |
| High-Pass | `dly_hp` | 20 – 2000 Hz | 20 Hz | Removes low frequencies from the repeats. Each repeat loses more. |
| Low-Pass | `dly_lp` | 500 – 20000 Hz | 12000 Hz | Removes high frequencies from the repeats. Each repeat gets darker. |
| Diffusion | `dly_diffusion` | 0.0 – 1.0 | 0.0 | Smears each repeat, so it sounds softer and less like a sharp echo. |
| Ducking | `dly_duck` | 0.0 – 1.0 | 0.0 | Lowers the repeats while the input plays. At 1.0 a loud input mutes them. |
| Wow | `dly_wow` | 0.0 – 1.0 | 0.3 | Slow pitch drift of the repeats. Used in `Tape` mode, and at half depth in `BBD` mode. |
| Drive | `dly_drive` | 0.0 – 1.0 | 0.3 | Saturation of the repeats. Used only in `Tape` mode. |

**Note values**: `1/1`, `1/2D`, `1/2`, `1/2T`, `1/4D`, `1/4`, `1/4T`, `1/8D`, `1/8`, `1/8T`, `1/16D`, `1/16`, `1/16T`, `1/32`. `D` means dotted (1.5 times longer). `T` means triplet (two thirds of the length).

**Tempo sync**: the plugin reads the tempo from the host. A synced time is still limited to 20 – 4000 ms. For example, `1/1` at 50 BPM would be 4800 ms, so it plays at 4000 ms.

#### Routing
- **Stereo**: the left input feeds the left delay. The right input feeds the right delay. Each side feeds back into itself.
- **Ping-Pong**: the plugin sums the input to mono and sends it to the left delay. The left delay feeds the right one, and the right feeds the left. The first repeat is on the left, the second on the right.

In `Ping-Pong`, **R Offset** makes every right repeat later by that amount.

#### Feedback Loop
In every mode, a repeat passes these steps before it goes back into the delay line:

1. **Diffusion**: two all-pass filters. An all-pass filter changes the timing of frequencies, not their level.
2. **High-Pass**, then **Low-Pass**.
3. **Mode color**: saturation in `Tape` mode, or a dark filter in `BBD` mode.
4. **Feedback** gain, then a soft clipper. A soft clipper rounds off loud peaks instead of cutting them hard.

The filters act again on every pass. So the fifth repeat is much darker and thinner than the first.

#### Ducking
1. The plugin follows the level of the dry input. It reacts in 5 ms and recovers in 250 ms.
2. While the input is loud, the level of the repeats goes down by the **Ducking** amount.
3. When the input stops, the repeats come back.

Ducking lowers only the output of the repeats. The repeats inside the loop keep their level. Ducking reaches its full amount when the input peaks at about -12 dBFS or higher.

#### Bypass
The header toggle of the `DELAY` card is a soft bypass:

1. The delay stops recording new input.
2. The loop keeps running, so the repeats that already exist fade out at their normal rate.
3. The dry signal returns to full level in about 30 ms.

#### Modes

**Clean**
- The repeats are exact copies of the input, shaped only by the loop filters.
- When the time changes, the plugin crossfades from the old time to the new one in 50 ms. The pitch does not change.
- **Wow** and **Drive** have no effect.

**Tape**
- **Drive** saturates the repeats. More drive makes them louder and denser.
- **Wow** adds a slow pitch drift (0.5 Hz, up to ±1.5 ms) and a fast one called flutter (6.3 Hz, up to ±0.15 ms).
- When the time changes, the delay glides to the new time in about 150 ms. The pitch of the repeats bends while it glides.
- The glide speed is limited. A big jump takes longer, and the pitch bends by 35% at most.

**BBD**
- BBD means bucket-brigade device. It is an old analog delay chip with a dark sound.
- An extra low-pass filter makes the repeats dark. It gets darker with longer times: about 5000 Hz at short times, 3500 Hz at 300 ms, 1200 Hz at the longest times.
- A light saturation is always on. **Drive** has no effect.
- **Wow** works at half depth.
- A time change glides and bends the pitch, like in `Tape` mode.

**Granular**
- The repeats are built from grains. A grain is a short piece of audio. The knobs of the [Grain](#grain) card shape them.
- **Time** sets how far back the grains read. New grains use the new time. Running grains finish at the old time.
- The grains pass the same diffusion, filters, feedback, and ducking as the other modes.
- **Wow** and **Drive** have no effect.

---

### Grain

Card title: `GRAIN`. This module is used only when the Delay **Mode** is `Granular`. It has no bypass toggle.

In `Granular` mode, the plugin does not play the delayed audio as one stream. It starts many short grains that each read a piece of the delay line. Each grain fades in and out with a Hann window (a smooth bell-shaped curve), so it does not click.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Size | `grain_size` | 10 – 500 ms | 80 ms | Length of each grain. Short grains sound rough. Long grains sound smooth. |
| Density | `grain_density` | 2 – 60 Hz | 20 Hz | Number of new grains per second. |
| Pitch | `grain_pitch` | -24 – +24 st | 0 st | Pitch shift of the grains in semitones. +12 is one octave up. |
| Spray | `grain_spray` | 0.0 – 1.0 | 0.1 | Random extra delay per grain. 1.0 is up to 200 ms. It blurs the timing of the repeats. |
| Reverse | `grain_reverse` | 0.0 – 1.0 | 0.0 | Chance that a grain plays backwards. 0.0 is never. 1.0 is always. |
| Spread | `grain_spread` | 0.0 – 1.0 | 0.5 | Random left-right position per grain. 0.0 puts every grain in the center. |

> **Note**: the engine has 32 voices. One voice plays one grain. When all 32 are busy, the next grain is skipped.

> **Note**: the number of grains that play at once is about **Density** × **Size**. For example, 60 Hz × 500 ms is 30 grains. The plugin lowers the level as this number grows, so the output stays steady.

> **Note**: the time between two grains varies by up to 25%. This avoids a buzzing tone at the density rate.

---

## 4. Global Controls

### Top Header Bar

- **Logo and subtitle**: shows `DELAY  //  CLEAN, TAPE, BBD & GRANULAR`.
- **Signal flow label**: shows `DELAY (+ GRAIN) -> MASTER`.
- **Preset Selector**: lists the 10 factory presets. The `<` and `>` buttons step through them.
- **DICE**: sets 14 knobs to random values inside safe limits. See below.
- **BYPASS**: master bypass. See `master_bypass` in the [Master Section](#master-section).

**What DICE changes**: **Time**, **Feedback**, **High-Pass**, **Low-Pass**, **Diffusion**, **Ducking**, **Wow**, **Drive**, and **Mix** on the Delay card. **Size**, **Density**, **Spray**, **Reverse**, and **Spread** on the Grain card.

**What DICE does not change**: the Delay bypass, **Mode**, **Routing**, **SYNC**, **Note**, **R Offset**, Grain **Pitch**, Modulation, and Master. DICE keeps **Feedback** well below 100%.

---

### Modulation: LFO 1 & LFO 2

The two LFOs sit in the lower part of the center column. An LFO (low-frequency oscillator) is a slow repeating wave that moves a parameter for you. Each LFO moves one target.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Wave (LFO 1) | `lfo1_wave` | Sine / Triangle / Saw Down / Saw Up / Square / Sample & Hold / Smooth Random | Sine | Waveform of LFO 1. |
| SYNC (LFO 1) | `lfo1_sync` | On / Off | Off | Off: free rate in Hz. On: rate follows the host tempo. |
| Rate (LFO 1) | `lfo1_rate` | 0.02 – 30 Hz | 1.0 Hz | Speed of LFO 1 when **SYNC** is off. |
| Sync Div (LFO 1) | `lfo1_sync_div` | 24 divisions (see below) | 1/4 | Cycle length of LFO 1 when **SYNC** is on. |
| Target (LFO 1) | `lfo1_target` | 14 choices (see below) | None | Parameter that LFO 1 moves. |
| Depth (LFO 1) | `lfo1_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the modulation. |
| Wave (LFO 2) | `lfo2_wave` | Same 7 waveforms | Sine | Waveform of LFO 2. |
| SYNC (LFO 2) | `lfo2_sync` | On / Off | Off | Off: free rate in Hz. On: rate follows the host tempo. |
| Rate (LFO 2) | `lfo2_rate` | 0.02 – 30 Hz | 1.0 Hz | Speed of LFO 2 when **SYNC** is off. |
| Sync Div (LFO 2) | `lfo2_sync_div` | 24 divisions (see below) | 1/4 | Cycle length of LFO 2 when **SYNC** is on. |
| Target (LFO 2) | `lfo2_target` | 14 choices (see below) | None | Parameter that LFO 2 moves. |
| Depth (LFO 2) | `lfo2_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the modulation. |

**Sync divisions**: `4/1`, `2/1`, `1/1`, `1/2`, `1/4`, `1/8`, `1/16`, `1/32`. Each one also exists as dotted (`D`) and triplet (`T`).

**Targets**: `None` plus 13 targets.

| # | Target | What moves | Reach at full depth |
| :--- | :--- | :--- | :--- |
| 1 | `Delay Time` | Delay time (free or synced) | 1 octave: from half to double the time |
| 2 | `Feedback` | Delay **Feedback** | ±1.0 |
| 3 | `LP Cutoff` | Delay **Low-Pass** | 3 octaves up or down |
| 4 | `HP Cutoff` | Delay **High-Pass** | 3 octaves up or down |
| 5 | `Diffusion` | Delay **Diffusion** | ±1.0 |
| 6 | `Wow` | Delay **Wow** | ±1.0 |
| 7 | `Delay Mix` | Delay **Mix** | ±1.0 |
| 8 | `Grain Size` | Grain **Size** | 2 octaves: from a quarter to four times the size |
| 9 | `Grain Pitch` | Grain **Pitch** | ±12 semitones |
| 10 | `Grain Spray` | Grain **Spray** | ±1.0 |
| 11 | `Master Mix` | Master **Dry/Wet** | ±1.0 |
| 12 | `LFO 2 Rate` | Speed of LFO 2 | 3 octaves up or down |
| 13 | `Master Vol` | Master **Volume** | ±24 dB |

The modulated value always stays inside the range of its parameter. The knob on screen does not move.

`LFO 2 Rate` works only as the target of LFO 1.

**Tempo sync**: while the host plays, a synced LFO is locked to the host beat position. While the host is stopped, it runs free at the host tempo.

**Delay Time as a target**: the result depends on the mode. In `Tape` and `BBD` mode the pitch of the repeats moves up and down. In `Clean` mode the plugin crossfades between times, so the pitch does not move.

---

### Modulation: X/Y Pad, Monitor & Taps

The top of the center column has three tabs: `X/Y PAD`, `MONITOR`, and `TAPS`.

**X/Y PAD tab** (card title `MOD & X/Y`): drag the puck to move two targets at once. The center of the pad means no change.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| Puck, horizontal | `xy_x` | 0.0 – 1.0 | 0.5 | Horizontal position of the puck. |
| Puck, vertical | `xy_y` | 0.0 – 1.0 | 0.5 | Vertical position of the puck. |
| X Target | `xy_x_target` | Same 14 choices as the LFOs | None | Parameter that the horizontal axis moves. |
| Y Target | `xy_y_target` | Same 14 choices as the LFOs | None | Parameter that the vertical axis moves. |
| X Depth | `xy_x_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the horizontal modulation. |
| Y Depth | `xy_y_depth` | -1.0 – +1.0 | 0.0 | Amount and direction of the vertical modulation. |

**MONITOR tab** (card title `MONITOR`): replaces the pad with a display that compares the input and the output signal. The plugin opens on this tab.

**TAPS tab** (card title `TAPS`): replaces the pad with the delay tap display. See [The TAPS Tab](#the-taps-tab).

---

### Master Section

Card title: `MASTER`. The BYPASS button is in the header bar.

| Control label | Parameter ID | Range | Default | What it does |
| :--- | :--- | :--- | :--- | :--- |
| In Gain | `input_gain` | -24 – +24 dB | 0 dB | Gain before the Delay. It also changes the dry signal. |
| Dry/Wet | `master_mix` | 0.0 – 1.0 | 1.0 | 0.0 is the dry signal only. 1.0 is the output of the Delay only. |
| Volume | `master_vol` | -36 – +12 dB | 0 dB | Output level. Applied before MONO and CLIP. |
| MONO | `master_mono_bass` | On / Off | Off | Sums everything below 120 Hz to mono. |
| CLIP | `master_clip` | On / Off | On | Hard clip at 0 dBFS on the output. |
| BYPASS (header) | `master_bypass` | On / Off | Off | Crossfades to the dry signal in 10 ms. |

The card also shows peak and RMS output meters.

> **Note**: the plugin has two mix controls. Delay **Mix** sets the level of the repeats. Master **Dry/Wet** is 1.0 by default and passes the Delay output unchanged. In most cases, use Delay **Mix** only.

> **Note**: the plugin has two bypass controls. The toggle of the `DELAY` card lets the repeats fade out. **BYPASS** in the header cuts the repeats in 10 ms.

---

## 5. Factory Presets

WvZz Delay has 10 factory presets. The plugin starts on preset 1.

| # | Preset | Use case | Mode |
| :--- | :--- | :--- | :--- |
| 1 | **Clean Quarter Note** | A standard tempo delay for vocals and leads. Synced to `1/4`, 35% feedback. | Clean |
| 2 | **Dotted Eighth Ping-Pong** | Rhythmic guitar and synth lines. Synced to `1/8D`, repeats alternate left and right, light ducking. | Clean |
| 3 | **Slapback Tape** | One short echo for vocals, guitars, and snares. 115 ms, 12% feedback, medium drive. | Tape |
| 4 | **Worn Tape Echo** | An old, unstable echo for keys and guitars. 380 ms, strong wow, dark low-pass at 4200 Hz. | Tape |
| 5 | **Dark BBD Chorus Echo** | Warm analog echo with pitch movement. 290 ms, **Wow** at 0.6, right side 8 ms later. | BBD |
| 6 | **Dub Feedback Wash** | Long dub echoes on snares, chords, and vocal words. Synced to `1/4D`, ping-pong, 92% feedback, ducked. | BBD |
| 7 | **Diffused Ambient Trail** | A soft, reverb-like trail for pads and slow parts. 620 ms, full diffusion, right side 25 ms later. | Clean |
| 8 | **Ducked Vocal Throw** | Echoes that appear after a vocal phrase ends. Synced to `1/2`, ping-pong, **Ducking** at 0.9. | Clean |
| 9 | **Granular Cloud** | A blurred texture behind pads and sustained notes. 140 ms grains, high spray, 25% reversed. | Granular |
| 10 | **Octave Shimmer Ladder** | Repeats that climb one octave each time. Synced to `1/8`, ping-pong, grain pitch +12 semitones. | Granular |

### How a Preset Loads
1. The plugin sets every parameter to its default. This includes Grain, Modulation, and Master.
2. The plugin applies the values that the preset stores.

So a preset always sounds the same, whatever you loaded before.

> **Note**: loading a preset also resets **In Gain**, **Volume**, **Dry/Wet**, the LFOs, and the X/Y pad. No factory preset uses LFO 1, LFO 2, or the X/Y pad.

---

## 6. Signal Flow & Routing

```text
[Main In] ──► [Input Gain] ──┬──────────────► (dry copy for Master)
                             │
                             ▼
                    [Delay]
                      dry ────────────────────────────────────┐
                      input + feedback ──► [Delay line]       │
                                                │             │
                                                ▼             │
                          [Tap read, or Grain engine]         │
                                                ▼             │
                          [Diffusion ► High-Pass ► Low-Pass]  │
                                                ▼             │
                          [Mode color (Tape, BBD)]            │
                                                │             │
                 feedback ◄── [Soft clip] ◄─────┤             │
                                                ▼             ▼
                                   [Ducking] ──► [Mix with the dry signal]
                             │
                             ▼
                    [Master: Dry/Wet ► Volume ► MONO ► CLIP ► Bypass]
                             │
                             ▼
                        [Stereo Out]
```

- The Delay is the only effect in the chain. The Grain module is part of the Delay in `Granular` mode.
- LFO 1, LFO 2, and the X/Y pad are computed first in every audio block.
- The left and the right channel each have their own delay line and feedback loop.
- The plugin has a stereo input and a stereo output. It has no sidechain input.

### Latency and Tail
- **Latency**: the plugin reports no latency to the host. The dry signal is not delayed.
- **Tail**: the tail is the time the plugin still makes sound after the input stops. The plugin reports it to the host.
- **Tail length**: the time until the repeats are 60 dB quieter. It follows the delay time and **Feedback**.
- **Shortest tail**: one delay time, at 0% feedback.
- **Longest tail**: 60 seconds. This value is used at 100% feedback or more, because the repeats do not decay there.
- **Example**: at 350 ms and 40% feedback, the tail is about 2.6 seconds.
- **Delay bypass**: the toggle of the `DELAY` card lets the tail fade out. See [Bypass](#bypass).

---

## 7. Usage Tips

- **Start with Time, Feedback, and Mix.** These three knobs set the basic delay. Add the other controls after that.
- **Use the tap display.** Open the `TAPS` tab and turn **Feedback**. You see how many repeats you get and how fast they fade.
- **Filter the repeats.** Raise **High-Pass** to about 150 Hz and lower **Low-Pass** to about 6000 Hz. The repeats then sit behind the dry sound.
- **Use ducking on vocals.** Set **Ducking** to 0.5 or more. The words stay clear, and the echoes fill the gaps.
- **Widen with R Offset.** A value of 10 to 25 ms moves the right repeats away from the left ones.
- **Make a slapback.** Use `Tape` mode, a **Time** of 80 to 130 ms, and **Feedback** near 0.1.
- **Soften with Diffusion.** High **Diffusion** with high **Feedback** turns the echoes into a reverb-like trail.
- **Play the Time knob in Tape or BBD mode.** The pitch bends while the time glides. Use `Clean` mode if you do not want this.
- **Go above 100% with care.** The repeats grow until the soft clipper holds them. Lower **Feedback** below 1.0 to let them fade.
- **Start Granular mode with Pitch at 0.** First set **Size**, **Density**, and **Spray**. Then add **Pitch** in steps of 12 or 7 semitones.
- **Use the plugin on a send.** Set Delay **Mix** to 1.0 so the send returns only the repeats.
- **Keep the low end steady.** Turn **MONO** on if wide repeats make the bass unstable.
- **Save your own settings in the host.** Loading a factory preset resets every parameter.

---

## 8. Troubleshooting & FAQ

**Q: Why is there no AU (Audio Unit) version?**  
A: WvZz Delay is built as VST3 and Standalone only. Use a VST3 host or the Standalone app.

**Q: Why is the minimum time 20 ms?**  
A: The two diffusion filters of each channel are always in the signal path. Together they are about 14 ms long on the left and 16 ms on the right. The plugin shortens the main delay by this length, so the total time stays exact. A total time below 20 ms would leave no room for that.

**Q: What happens above 100% feedback?**  
A: Each repeat is louder than the one before, so the repeats do not fade. A soft clipper in the loop limits the level. The sound gets more saturated, but it does not grow without limit. The reported tail is 60 seconds. Lower **Feedback** below 1.0 or bypass the Delay card to end it.

**Q: Why do pitch-shifted grains climb with each repeat?**  
A: The pitch shift is inside the feedback loop. The first repeat is shifted once. It goes back into the delay line and is shifted again on the next pass. With **Pitch** at +12, the repeats are one, two, then three octaves up. Set **Feedback** to 0.0 for a single shifted repeat.

**Q: Why does the Time knob do nothing?**  
A: **SYNC** is on, so the time comes from **Note** and the host tempo. The **Time** knob is dimmed. Turn **SYNC** off.

**Q: Why do Wow and Drive do nothing?**  
A: **Wow** works only in `Tape` and `BBD` mode. **Drive** works only in `Tape` mode. A dimmed knob is not used by the current mode.

**Q: Why do the Grain knobs do nothing?**  
A: The Grain module is used only when the Delay **Mode** is `Granular`.

**Q: Why does the pitch change when I turn Time?**  
A: In `Tape` and `BBD` mode the delay glides to the new time, like a tape machine that changes speed. Use `Clean` mode for a time change without a pitch change.

**Q: Why are the repeats quiet while I play?**  
A: **Ducking** is above 0.0. It lowers the repeats while the input is loud. Lower **Ducking** to hear them all the time.

**Q: Why is my stereo input mono in the repeats?**  
A: **Routing** is `Ping-Pong`. This routing sums the input to mono before the delay. Use `Stereo` to keep left and right separate.

**Q: Why is a synced delay shorter than the note value?**  
A: The delay time is limited to 4000 ms. A long note value at a slow tempo can be longer than that.

**Q: Why did my Volume and LFO settings change when I picked a preset?**  
A: A preset resets every parameter to its default before it applies its values. This includes Master and Modulation.

**Q: Why does the sound change when I move the X/Y pad?**  
A: An axis has a target and a depth. Set **X Depth** and **Y Depth** to 0.0 to turn the pad off.
